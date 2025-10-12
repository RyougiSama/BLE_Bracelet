/**
 * @file max30102_algorithm.c
 * @brief Heart rate and SpO2 calculation algorithm implementation
 * @author Converted from Maxim Integrated reference design
 * @date 2025
 */

/* Includes ------------------------------------------------------------------*/
#include "max30102_algorithm.h"

#include <string.h>

/* Private Constants ---------------------------------------------------------*/

/**
 * @brief Hamming window coefficients for signal filtering
 * Hamming window size = 5, coefficients scaled by 512
 */
const uint16_t hamming_window[ALGORITHM_HAMMING_SIZE] = {41, 276, 512, 276, 41};

/**
 * @brief SpO2 lookup table
 * Pre-calculated SpO2 values based on ratio: SpO2 = -45.060*ratio^2 + 30.354*ratio + 94.845
 * Index represents the ratio value, table value is the corresponding SpO2 percentage
 */
const uint8_t spo2_lookup_table[184] = {
    95,  95,  95,  96,  96,  96,  97,  97,  97,  97,  97,  98,  98,  98,  98,  98,  99,  99,  99,
    99,  99,  99,  99,  99,  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 99,  99,  99,  99,  99,  99,  99,  99,  98,  98,  98,  98,  98,
    98,  97,  97,  97,  97,  96,  96,  96,  96,  95,  95,  95,  94,  94,  94,  93,  93,  93,  92,
    92,  92,  91,  91,  90,  90,  89,  89,  89,  88,  88,  87,  87,  86,  86,  85,  85,  84,  84,
    83,  82,  82,  81,  81,  80,  80,  79,  78,  78,  77,  76,  76,  75,  74,  74,  73,  72,  72,
    71,  70,  69,  69,  68,  67,  66,  66,  65,  64,  63,  62,  62,  61,  60,  59,  58,  57,  56,
    56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,
    37,  36,  35,  34,  33,  31,  30,  29,  28,  27,  26,  25,  23,  22,  21,  20,  19,  17,  16,
    15,  14,  12,  11,  10,  9,   7,   6,   5,   3,   2,   1};

/* Private Variables ---------------------------------------------------------*/
static algorithm_buffers_t algorithm_buffers;

/* Private Function Prototypes -----------------------------------------------*/
static void apply_moving_average_filter(int32_t *signal, int32_t size, int32_t window_size);
static void apply_hamming_filter(int32_t *signal, int32_t size);
static int32_t calculate_threshold(int32_t *signal, int32_t size);

/* Public Functions ----------------------------------------------------------*/

/**
 * @brief Initialize algorithm buffers
 */
void Algorithm_InitBuffers(algorithm_buffers_t *buffers)
{
    if (buffers != NULL) {
        memset(buffers, 0, sizeof(algorithm_buffers_t));
    }
}

/**
 * @brief Get SpO2 value from lookup table
 */
int32_t Algorithm_GetSpO2FromTable(int32_t ratio_average)
{
    if (ratio_average >= 2 && ratio_average < 184) {
        return spo2_lookup_table[ratio_average];
    }
    return -999;  // Invalid ratio
}

/**
 * @brief Calculate heart rate and SpO2 from PPG signals
 */
void Algorithm_CalculateHeartRateAndSpO2(uint32_t *ir_buffer, int32_t buffer_length,
                                         uint32_t *red_buffer, algorithm_results_t *results)
{
    // Local variables
    uint32_t ir_mean = 0;
    int32_t i, k;
    int32_t num_peaks = 0;
    int32_t peak_locations[ALGORITHM_MAX_PEAKS];
    int32_t valley_locations[ALGORITHM_MAX_PEAKS];
    int32_t exact_valleys[ALGORITHM_MAX_PEAKS];
    int32_t num_exact_valleys = 0;
    int32_t peak_interval_sum = 0;
    int32_t threshold;

    // SpO2 calculation variables
    int32_t y_ac, x_ac, y_dc_max, x_dc_max;
    int32_t y_dc_max_idx, x_dc_max_idx;
    int32_t ratios[5];
    int32_t ratio_count = 0;
    int32_t ratio_average = 0;
    int32_t numerator, denominator;

    // Initialize results
    results->heart_rate = -999;
    results->spo2 = -999;
    results->hr_valid = false;
    results->spo2_valid = false;

    if (buffer_length > ALGORITHM_BUFFER_SIZE) {
        buffer_length = ALGORITHM_BUFFER_SIZE;
    }

    // Step 1: Remove DC component from IR signal
    ir_mean = 0;
    for (k = 0; k < buffer_length; k++) {
        ir_mean += ir_buffer[k];
    }
    ir_mean = ir_mean / buffer_length;

    for (k = 0; k < buffer_length; k++) {
        algorithm_buffers.ir_buffer[k] = (int32_t)ir_buffer[k] - ir_mean;
    }

    // Step 2: Apply 4-point moving average filter
    apply_moving_average_filter(algorithm_buffers.ir_buffer, buffer_length, ALGORITHM_MA4_SIZE);

    // Step 3: Calculate first derivative
    for (k = 0; k < buffer_length - ALGORITHM_MA4_SIZE - 1; k++) {
        algorithm_buffers.dx_buffer[k] =
            algorithm_buffers.ir_buffer[k + 1] - algorithm_buffers.ir_buffer[k];
    }

    // Step 4: Apply 2-point moving average to derivative
    apply_moving_average_filter(algorithm_buffers.dx_buffer, buffer_length - ALGORITHM_MA4_SIZE - 1,
                                2);

    // Step 5: Apply Hamming window filter
    apply_hamming_filter(algorithm_buffers.dx_buffer, buffer_length - ALGORITHM_MA4_SIZE - 2);

    // Step 6: Calculate adaptive threshold
    threshold =
        calculate_threshold(algorithm_buffers.dx_buffer, buffer_length - ALGORITHM_HAMMING_SIZE);

    // Step 7: Find peaks in filtered signal
    Algorithm_FindPeaks(peak_locations, &num_peaks, algorithm_buffers.dx_buffer,
                        buffer_length - ALGORITHM_HAMMING_SIZE, threshold, 8, 5);

    // Step 8: Calculate heart rate from peak intervals
    if (num_peaks >= 2) {
        peak_interval_sum = 0;
        for (k = 1; k < num_peaks; k++) {
            peak_interval_sum += (peak_locations[k] - peak_locations[k - 1]);
        }
        peak_interval_sum = peak_interval_sum / (num_peaks - 1);
        results->heart_rate = (int32_t)(6000 / peak_interval_sum);  // Convert to BPM
        results->hr_valid = true;
    }

    // Step 9: Convert peak locations to valley locations for SpO2 calculation
    for (k = 0; k < num_peaks; k++) {
        valley_locations[k] = peak_locations[k] + ALGORITHM_HAMMING_SIZE / 2;
    }

    // Step 10: Copy raw signals for SpO2 processing
    for (k = 0; k < buffer_length; k++) {
        algorithm_buffers.ir_buffer[k] = (int32_t)ir_buffer[k];
        algorithm_buffers.red_buffer[k] = (int32_t)red_buffer[k];
    }

    // Step 11: Find precise valley locations
    num_exact_valleys = 0;
    for (k = 0; k < num_peaks; k++) {
        int32_t valley_pos = valley_locations[k];
        int32_t min_val = 16777216;  // Large initial value
        bool valley_found = false;

        if (valley_pos + 5 < buffer_length - ALGORITHM_HAMMING_SIZE && valley_pos - 5 > 0) {
            for (i = valley_pos - 5; i < valley_pos + 5; i++) {
                if (algorithm_buffers.ir_buffer[i] < min_val) {
                    min_val = algorithm_buffers.ir_buffer[i];
                    exact_valleys[num_exact_valleys] = i;
                    valley_found = true;
                }
            }
            if (valley_found) {
                num_exact_valleys++;
            }
        }
    }

    if (num_exact_valleys < 2) {
        return;  // Not enough valleys for SpO2 calculation
    }

    // Step 12: Apply moving average to raw signals
    apply_moving_average_filter(algorithm_buffers.ir_buffer, buffer_length, ALGORITHM_MA4_SIZE);
    apply_moving_average_filter(algorithm_buffers.red_buffer, buffer_length, ALGORITHM_MA4_SIZE);

    // Step 13: Calculate AC/DC ratios between valleys
    for (k = 0; k < 5; k++) {
        ratios[k] = 0;
    }

    for (k = 0; k < num_exact_valleys - 1; k++) {
        if (exact_valleys[k + 1] - exact_valleys[k] > 10) {
            y_dc_max = -16777216;
            x_dc_max = -16777216;

            // Find maximum values between valleys
            for (i = exact_valleys[k]; i < exact_valleys[k + 1]; i++) {
                if (algorithm_buffers.ir_buffer[i] > x_dc_max) {
                    x_dc_max = algorithm_buffers.ir_buffer[i];
                    x_dc_max_idx = i;
                }
                if (algorithm_buffers.red_buffer[i] > y_dc_max) {
                    y_dc_max = algorithm_buffers.red_buffer[i];
                    y_dc_max_idx = i;
                }
            }

            // Calculate AC components (subtract linear DC trend)
            y_ac = (algorithm_buffers.red_buffer[exact_valleys[k + 1]] -
                    algorithm_buffers.red_buffer[exact_valleys[k]]) *
                   (y_dc_max_idx - exact_valleys[k]);
            y_ac = algorithm_buffers.red_buffer[exact_valleys[k]] +
                   y_ac / (exact_valleys[k + 1] - exact_valleys[k]);
            y_ac = algorithm_buffers.red_buffer[y_dc_max_idx] - y_ac;

            x_ac = (algorithm_buffers.ir_buffer[exact_valleys[k + 1]] -
                    algorithm_buffers.ir_buffer[exact_valleys[k]]) *
                   (x_dc_max_idx - exact_valleys[k]);
            x_ac = algorithm_buffers.ir_buffer[exact_valleys[k]] +
                   x_ac / (exact_valleys[k + 1] - exact_valleys[k]);
            x_ac = algorithm_buffers.ir_buffer[y_dc_max_idx] - x_ac;

            // Calculate ratio: (Red_AC/Red_DC) / (IR_AC/IR_DC)
            numerator = (y_ac * x_dc_max) >> 7;  // Scale to prevent overflow
            denominator = (x_ac * y_dc_max) >> 7;

            if (denominator > 0 && ratio_count < 5 && numerator != 0) {
                ratios[ratio_count] = (numerator * 100) / denominator;
                ratio_count++;
            }
        }
    }

    // Step 14: Calculate median ratio
    if (ratio_count > 0) {
        Algorithm_SortAscending(ratios, ratio_count);
        int32_t middle_idx = ratio_count / 2;

        if (middle_idx > 1) {
            ratio_average = (ratios[middle_idx - 1] + ratios[middle_idx]) / 2;
        } else {
            ratio_average = ratios[middle_idx];
        }

        // Step 15: Convert ratio to SpO2 using lookup table
        results->spo2 = Algorithm_GetSpO2FromTable(ratio_average);
        if (results->spo2 != -999) {
            results->spo2_valid = true;
        }
    }
}

/**
 * @brief Find peaks in signal with specified criteria
 */
void Algorithm_FindPeaks(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                         int32_t signal_size, int32_t min_height, int32_t min_distance,
                         int32_t max_peaks)
{
    Algorithm_FindPeaksAboveHeight(locations, num_peaks, signal, signal_size, min_height);
    Algorithm_RemoveClosePeaks(locations, num_peaks, signal, min_distance);
    *num_peaks = ALGORITHM_MIN(*num_peaks, max_peaks);
}

/**
 * @brief Find all peaks above minimum height
 */
void Algorithm_FindPeaksAboveHeight(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                                    int32_t signal_size, int32_t min_height)
{
    int32_t i = 1;
    int32_t width;
    *num_peaks = 0;

    while (i < signal_size - 1) {
        if (signal[i] > min_height && signal[i] > signal[i - 1]) {
            width = 1;
            while (i + width < signal_size && signal[i] == signal[i + width]) {
                width++;
            }
            if (signal[i] > signal[i + width] && (*num_peaks) < ALGORITHM_MAX_PEAKS) {
                locations[(*num_peaks)++] = i;
                i += width + 1;
            } else {
                i += width;
            }
        } else {
            i++;
        }
    }
}

/**
 * @brief Remove peaks that are too close together
 */
void Algorithm_RemoveClosePeaks(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                                int32_t min_distance)
{
    int32_t i, j, old_num_peaks, distance;

    // Sort peaks by amplitude (descending)
    Algorithm_SortIndicesDescending(signal, locations, *num_peaks);

    for (i = -1; i < *num_peaks; i++) {
        old_num_peaks = *num_peaks;
        *num_peaks = i + 1;

        for (j = i + 1; j < old_num_peaks; j++) {
            distance = locations[j] - (i == -1 ? -1 : locations[i]);
            if (distance > min_distance || distance < -min_distance) {
                locations[(*num_peaks)++] = locations[j];
            }
        }
    }

    // Resort locations in ascending order
    Algorithm_SortAscending(locations, *num_peaks);
}

/**
 * @brief Sort array in ascending order using insertion sort
 */
void Algorithm_SortAscending(int32_t *array, int32_t size)
{
    int32_t i, j, temp;

    for (i = 1; i < size; i++) {
        temp = array[i];
        for (j = i; j > 0 && temp < array[j - 1]; j--) {
            array[j] = array[j - 1];
        }
        array[j] = temp;
    }
}

/**
 * @brief Sort indices according to descending signal values
 */
void Algorithm_SortIndicesDescending(int32_t *signal, int32_t *indices, int32_t size)
{
    int32_t i, j, temp;

    for (i = 1; i < size; i++) {
        temp = indices[i];
        for (j = i; j > 0 && signal[temp] > signal[indices[j - 1]]; j--) {
            indices[j] = indices[j - 1];
        }
        indices[j] = temp;
    }
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Apply moving average filter to signal
 */
static void apply_moving_average_filter(int32_t *signal, int32_t size, int32_t window_size)
{
    int32_t i, j, sum;

    for (i = 0; i < size - window_size; i++) {
        sum = 0;
        for (j = 0; j < window_size; j++) {
            sum += signal[i + j];
        }
        signal[i] = sum / window_size;
    }
}

/**
 * @brief Apply Hamming window filter
 */
static void apply_hamming_filter(int32_t *signal, int32_t size)
{
    int32_t i, k, sum;

    for (i = 0; i < size - ALGORITHM_HAMMING_SIZE - ALGORITHM_MA4_SIZE - 2; i++) {
        sum = 0;
        for (k = i; k < i + ALGORITHM_HAMMING_SIZE; k++) {
            sum -= signal[k] * hamming_window[k - i];
        }
        signal[i] = sum / 1146;  // Normalize by sum of hamming coefficients
    }
}

/**
 * @brief Calculate adaptive threshold for peak detection
 */
static int32_t calculate_threshold(int32_t *signal, int32_t size)
{
    int32_t k, threshold = 0;

    for (k = 0; k < size; k++) {
        threshold += (signal[k] > 0) ? signal[k] : -signal[k];  // Absolute value
    }

    return threshold / size;
}