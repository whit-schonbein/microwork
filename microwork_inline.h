/*****************************************************************************
 *
 * microwork_inline.h
 *
 * Author: Whit Schonbein
 * Email: whit.schonbein@gmail.com
 * Date: 2016
 * 
 * Written under auspices of Dorian Arnold and the University of New Mexico
 *
 *****************************************************************************/

#if !defined( __MICROWORK_INLINE_H_ )
#define __MICROWORK_INLINE_H_

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* if running on OSX/Mach kernel, requires different clock */
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/* include the work loops */
#include "microwork_inline_work.h"

/* calibration results */
typedef struct c_results_s {
  double average;
  double std_dev;
  uint64_t min;
  uint64_t max;
  uint64_t calibration_cycles;  /* number of cycles used for each calibration trial */
  uint64_t target_nsec; /* number of nsecs used to calculate loop_num */
  uint64_t loop_num;    /* number of loops (MXM) or cycles (ASM) required to elapse target_nsec nanoseconds */ 
} c_results_t;

/* rest types */
typedef enum rest_e {REST_SLEEP, REST_DEV_NULL} rest_t;

/* for conversion */
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_USEC 1000

/* when using rest method besides sleep(1), perform the wait using this number of iterations */
#define SLEEP_CYCLES 10000000

/*******************************************************************
 * UTILITY METHODS
 *******************************************************************/

/* subtract two timespecs */
uint64_t timespec_sub( struct timespec *a, struct timespec *b);

/* Calculate statistics for array of nsec timings.
 *
 * data     : array of nanoseconds or cycles (uint64_t)
 * length   : size of array
 * c_results: c_result_t struct for returning results
 */
void calc_stats(const uint64_t *data, int length, c_results_t *c_results);

/* Calculate estimated number of iterations (MXM) or cycles (ASM).
 *
 * target_nsec  : desired duration of work
 * c_results    : c_results_t struct containing results of calibration
 */
uint64_t calc_loop_num(uint64_t target_nsec, c_results_t *c_results);

/* Strip data points outside of num_std_dev standard deviations from in_data.
 *
 * in_data      : array of data to process
 * in_length    : size of input array
 * num_std_dev  : strip everything outside this many std deviations
 * verbose      : yell about what's going on
 * out_data     : array containing retained data. should be same size as in_data.
 *
 * Returns: number of elements in out_data.
 */ 
int strip_std_dev( const uint64_t *in_data, int in_length, int num_std_dev, int verbose, uint64_t *out_data);

/*******************************************************************
 * CALIBRATION (configured at compile time)
 *******************************************************************/

void calibrate(int num_trials, uint64_t cycles_per_trial, rest_t rest_type, int versbose, c_results_t *c_results);

/*******************************************************************
 * RESTING METHODS
 *******************************************************************/

void rest_dev_null(uint32_t iters);

#endif  /* __MICROWORK_INLINE_H_ */
