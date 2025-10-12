/**
 * @file max30102_algorithm.h
 * @brief Heart rate and SpO2 calculation algorithm header
 * @author Converted from Maxim Integrated reference design
 * @date 2025
 *
 * This module calculates heart rate and SpO2 level using PPG signal processing
 */

#ifndef __MAX30102_ALGORITHM_H
#define __MAX30102_ALGORITHM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Algorithm Configuration ---------------------------------------------------*/
#define ALGORITHM_FS 100                          // Sampling frequency (Hz)
#define ALGORITHM_BUFFER_SIZE (ALGORITHM_FS * 5)  // 5 seconds of data
#define ALGORITHM_HR_FIFO_SIZE 7
#define ALGORITHM_MA4_SIZE 4      // 4-point moving average
#define ALGORITHM_HAMMING_SIZE 5  // Hamming window size
#define ALGORITHM_MAX_PEAKS 15    // Maximum number of peaks to detect

/* Macro Definitions ---------------------------------------------------------*/
#define ALGORITHM_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ALGORITHM_MAX(x, y) ((x) > (y) ? (x) : (y))

/* Data Structures -----------------------------------------------------------*/

/**
 * @brief Algorithm calculation results structure
 */
typedef struct {
    int32_t heart_rate; /**< Calculated heart rate in BPM */
    int32_t spo2;       /**< Calculated SpO2 percentage */
    bool hr_valid;      /**< Heart rate validity flag */
    bool spo2_valid;    /**< SpO2 validity flag */
} algorithm_results_t;

/**
 * @brief Algorithm internal buffers structure
 */
typedef struct {
    int32_t ir_buffer[ALGORITHM_BUFFER_SIZE];                      /**< IR signal buffer */
    int32_t red_buffer[ALGORITHM_BUFFER_SIZE];                     /**< Red signal buffer */
    int32_t dx_buffer[ALGORITHM_BUFFER_SIZE - ALGORITHM_MA4_SIZE]; /**< Derivative buffer */
} algorithm_buffers_t;

/* External Constants --------------------------------------------------------*/
extern const uint16_t hamming_window[ALGORITHM_HAMMING_SIZE];
extern const uint8_t spo2_lookup_table[184];

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief Calculate heart rate and SpO2 from PPG signals
 * @param ir_buffer Pointer to IR LED data buffer
 * @param buffer_length Length of the data buffer
 * @param red_buffer Pointer to Red LED data buffer
 * @param results Pointer to store calculation results
 * @retval None
 */
void Algorithm_CalculateHeartRateAndSpO2(uint32_t *ir_buffer, int32_t buffer_length,
                                         uint32_t *red_buffer, algorithm_results_t *results);

/**
 * @brief Find peaks in a signal
 * @param locations Array to store peak locations
 * @param num_peaks Pointer to store number of peaks found
 * @param signal Input signal array
 * @param signal_size Size of input signal
 * @param min_height Minimum peak height threshold
 * @param min_distance Minimum distance between peaks
 * @param max_peaks Maximum number of peaks to find
 * @retval None
 */
void Algorithm_FindPeaks(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                         int32_t signal_size, int32_t min_height, int32_t min_distance,
                         int32_t max_peaks);

/**
 * @brief Find peaks above minimum height
 * @param locations Array to store peak locations
 * @param num_peaks Pointer to store number of peaks found
 * @param signal Input signal array
 * @param signal_size Size of input signal
 * @param min_height Minimum peak height
 * @retval None
 */
void Algorithm_FindPeaksAboveHeight(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                                    int32_t signal_size, int32_t min_height);

/**
 * @brief Remove peaks that are too close together
 * @param locations Array of peak locations
 * @param num_peaks Pointer to number of peaks
 * @param signal Input signal array
 * @param min_distance Minimum distance between peaks
 * @retval None
 */
void Algorithm_RemoveClosePeaks(int32_t *locations, int32_t *num_peaks, int32_t *signal,
                                int32_t min_distance);

/**
 * @brief Sort array in ascending order
 * @param array Array to sort
 * @param size Size of array
 * @retval None
 */
void Algorithm_SortAscending(int32_t *array, int32_t size);

/**
 * @brief Sort indices according to descending signal values
 * @param signal Signal array
 * @param indices Index array to sort
 * @param size Size of arrays
 * @retval None
 */
void Algorithm_SortIndicesDescending(int32_t *signal, int32_t *indices, int32_t size);

/**
 * @brief Initialize algorithm buffers
 * @param buffers Pointer to algorithm buffers structure
 * @retval None
 */
void Algorithm_InitBuffers(algorithm_buffers_t *buffers);

/**
 * @brief Get SpO2 value from lookup table
 * @param ratio_average Calculated ratio value
 * @retval SpO2 percentage (0-100) or -999 if invalid
 */
int32_t Algorithm_GetSpO2FromTable(int32_t ratio_average);

#ifdef __cplusplus
}
#endif

#endif /* MAX30102_ALGORITHM_H */