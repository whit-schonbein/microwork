/*****************************************************************************
 *
 * microwork_inline.c
 *
 * A collection of busy-wait work loops for modeling workload latency.
 *
 * Author: Whit Schonbein
 * Date: 2016
 * 
 * Written under auspices of Dorian Arnold and the University of New Mexico
 *
 * Matrix multiplication work loop based on code by Samuel Guiterrez and 
 * included in versions of his MRNetBench benchmarking tool.
 *
 * Assembly code informed by Intel document "How to benchmark code 
 * execution times on Intel IA-32 and IA-64 instruction set architectures" 
 *
 *****************************************************************************/

#include "microwork_inline.h"

/*****************************************************************************
 * CALIBRATION 
 *****************************************************************************/

/* 
 * num_trials : number of trials to use during calibration
 * cycles_per_trial : number of cycles to use for each trial. ignored 
 *    if work is MXM
 * rest_type : type of rest to perform between trials
 * verbose : if > 1, then babble
 * stats_ptr : pointer to stats struct wherein to store results
 *
 */
void calibrate(int num_trials, uint64_t cycles_per_trial, rest_t rest_type, int verbose, c_results_t *c_results_ptr) {
  
  int t;

  /* for measuring times */
  struct timespec start,end;
  /* mach/osx uses a different method to get times */
  #if defined(__MACH__)
    clock_serv_t cclock;
    mach_timespec_t mts_start, mts_end;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  #endif

  /* fill in default results */
  c_results_ptr->average = 0.0;
  c_results_ptr->std_dev = 0.0;
  c_results_ptr->min = 0;
  c_results_ptr->max = 0;
  c_results_ptr->calibration_cycles = cycles_per_trial;
  c_results_ptr->target_nsec = 0;
  c_results_ptr->loop_num = 0;

  #if defined( WORK_NULL )
    uint64_t *results = NULL;
    return;
  #elif defined( WORK_MXM )
    int loop_num = 1; /* calibrate on a single matrix multiplication per trial */
    uint64_t *results = (uint64_t *)malloc(num_trials * sizeof(*results)); 
  #elif defined( WORK_ASM_NOP ) || defined( WORK_ASM_MUL ) || defined( WORK_ASM_FADD ) || defined( WORK_ASM_FMUL )
    int loop_num = cycles_per_trial; /* calibrate on cycles_per_trial per trial */
    uint64_t *results = (uint64_t *)malloc(num_trials * sizeof(*results)); 
  #else
    /* the variable defined during compilation was not recognized, so print a warning and treat it like WORK_NULL */
    uint64_t *results = NULL;
    fprintf(stderr, "%s:%d: ERROR -- unknown work type.\n", __FILE__, __LINE__);
    return;
  #endif

  /* perform the calibration */
  /* Note this loop does not retain the results of the first trail; 
     This result is typically shorter than the others, so we discard it. */
  for (t = 0; t < num_trials+1; t++) {

    /* get start of trial timestamp */
    #if defined(__MACH__)
      clock_get_time(cclock, &mts_start);
    #else
      if ( clock_gettime( CLOCK_MONOTONIC, &start ) == -1 ) {
        fprintf(stderr, "%s:%d: Failure getting start clock time.\n", __FILE__, __LINE__);
        return(-1);
      }
    #endif

    #if defined( WORK_NULL )
      WORK_NULL_C
    #elif defined( WORK_MXM )
      WORK_MXM_C
    #elif defined( WORK_ASM_NOP )
      WORK_ASM_NOP_C
    #elif defined( WORK_ASM_MUL )
      WORK_ASM_MUL_C
    #elif defined( WORK_ASM_FADD )
      WORK_ASM_FADD_C
    #elif defined( WORK_ASM_FMUL )
      WORK_ASM_FMUL_C
    #else
      fprintf(stderr, "%s:%d: ERROR -- unknown work type.\n", __FILE__, __LINE__);
      WORK_NULL_C
    #endif

    /* get end of trial timestamp */
    #if defined(__MACH__)
      clock_get_time(cclock, &mts_end);
      start.tv_sec = mts_start.tv_sec;
      start.tv_nsec = mts_start.tv_nsec;
      end.tv_sec = mts_end.tv_sec;
      end.tv_nsec = mts_end.tv_nsec;
    #else
      if ( clock_gettime( CLOCK_MONOTONIC, &end ) == -1 ) {
        fprintf(stderr, "%s:%d: Failure getting end clock time.\n", __FILE__, __LINE__);
        return(-1);
      }
    #endif
    
    if (t > 0) {
      results[t-1] = timespec_sub(&start, &end);
      if (verbose) printf("Calibration Trial %d: %lld\t", t, results[t-1]);
    }

    /* rest */
    switch (rest_type) {
      case REST_SLEEP:
        if (verbose && t > 0) printf("(sleep(1))\n");
        sleep(1);
        break;
      case REST_DEV_NULL:
        if (verbose && t > 0) printf("(rest_dev_null(...))\n");
        rest_dev_null(SLEEP_CYCLES);
        break;
      default:
        fprintf(stderr, "%s:%d: ERROR -- unknown rest type.\n", __FILE__, __LINE__);
        sleep(1);
    }
  }

  /* clean up mach clock if needed */
  #if defined(__MACH__)
    mach_port_deallocate(mach_task_self(), cclock);
  #endif
  
  /* fill in results structure */
  calc_stats(results, num_trials, c_results_ptr);

  free(results);
}
  
/*******************************************************************
 * RESTING METHODS in addition to sleep(1)
 *******************************************************************/

/* rest by printing to dev/null */
void rest_dev_null( uint32_t iters ) {
  uint64_t j;    
  uint64_t k = 0;
  FILE *black_hole = fopen("/dev/null", "w");
  for (j = 0; j < iters; j++) {
    fprintf(black_hole, "This is the current number: %d\n", (j + (rand() % 500)));
    k = k + 1;
  }
  fclose(black_hole);
}

/*******************************************************************
 * UTILITY METHODS
 *******************************************************************/

/* 
 * Subtract two timespecs.
 * In:  timespecs a (start) and b (end)
 * Out: nanoseconds difference b - a
 */
uint64_t timespec_sub( struct timespec *a, struct timespec *b)
{
  uint64_t ret = (NSEC_PER_SEC * b->tv_sec) + b->tv_nsec;
  ret = ret - ((NSEC_PER_SEC * a->tv_sec) + a->tv_nsec);
  return ret;
}

/* 
 * Calculate statistics for array of nsec timings 
 */
void calc_stats(const uint64_t *data, int length, c_results_t *c_results_ptr) {
  uint64_t sum = 0, min, max;
  double v = 0.0;
  int i;
 
  for (i = 0; i < length; i++) sum += data[i];
  c_results_ptr->average = sum/(double)length;

  /* get min, max, and compute sum of squared differences from mean */
  c_results_ptr->min = (uint64_t)c_results_ptr->average;
  c_results_ptr->max = (uint64_t)c_results_ptr->average;
  for (i = 0; i < length; i++ ) {
    v += pow((data[i] - c_results_ptr->average), 2);
    if (c_results_ptr->min > data[i]) c_results_ptr->min = data[i];
    if (c_results_ptr->max < data[i]) c_results_ptr->max = data[i];
  }
  c_results_ptr->std_dev = sqrt(v / (float)length);
}

/*
 * Calculate loop_num for new target_nsec using previously-calculated 
 * calibration values.
 *
 * loop_num is both stored in the c_results_t structure and returned
 * the target_nsec used to calculate it is stored in c_results_t structure
 *
 */
uint64_t calc_loop_num(uint64_t target_nsec, c_results_t *c_results_ptr) {
  
  /* store requested number of nsecs */
  c_results_ptr->target_nsec = target_nsec;

  /* If compiled with WORK_NULL, then return 0 */
  #if defined( WORK_NULL )
    c_results_ptr->loop_num = 0;
    return 0;
  #endif

  /* if target is 0, return 0 */
  if(0 == target_nsec) {
    c_results_ptr->loop_num = 0;
    return 0;
  }
  
  /* truncate average nsec */
  uint64_t avg_nsec = c_results_ptr->average;

  /* if average is 0, return 0 */
  if(0 == avg_nsec) {
    fprintf(stderr, "%s:%d : WARNING: avg_nsec per trial == 0; returning 0 cycles for work loop\n", __FILE__, __LINE__);
    c_results_ptr->loop_num = 0;
    return 0;
  }

  #if defined( WORK_MXM )
    /* Since MXM is calibrated using a single matrix multiplication ... */
    c_results_ptr->loop_num = target_nsec/avg_nsec;
  #elif defined( WORK_ASM_NOP ) || defined( WORK_ASM_MUL ) || defined( WORK_ASM_MUL ) || defined( WORK_ASM_FADD ) || defined( WORK_ASM_FMUL )
    /* Assumption: number of cycles is a linear function of the time requested */
    c_results_ptr->loop_num = (uint64_t)((target_nsec/(long double)avg_nsec) * c_results_ptr->calibration_cycles);
  #else
    /* unknown work type */
    fprintf(stderr, "%s:%d: ERROR -- unknown work type.\n", __FILE__, __LINE__);
    exit(-1);
  #endif

  return c_results_ptr->loop_num;
}

/* 
 * Strip data points outside of num_std_dev standard deviations from in_data. 
 */
int strip_std_dev(const uint64_t *in_data, int in_length, int num_std_dev, int verbose, uint64_t *out_data)
{
  double sum = 0, average, std_dev;
  uint64_t usum = 0;
  int i, out_length;

  if (verbose) fprintf(stdout, "Stripping data points outside %d standard deviations.\n", num_std_dev);

  for (i = 0; i < in_length; i++) {
    usum += in_data[i];
  }
  average = usum/(double)in_length;

  sum = 0;
  for (i = 0; i < in_length; i++ ) {
    sum += pow((in_data[i] - average), 2);
  }
  std_dev = sqrt(sum / (double)in_length);

  if (verbose) fprintf(stdout, "The following are outside %d std devs: ", num_std_dev);

  /* copy items within num_std_dev * std_dev to output array */
  out_length = 0;
  for (i = 0; i < in_length; i++) {
    if ( ! (in_data[i] > (average + ( num_std_dev * std_dev)) || in_data[i] < (average - (num_std_dev * std_dev))) || num_std_dev == 0) {
      out_data[out_length] = in_data[i];
      ++out_length;
    } else if (verbose) fprintf(stdout, "\t%lld", in_data[i]);
  }
  if (verbose && out_length == in_length) fprintf(stdout, "\tNo values stripped.");
  if (verbose) fprintf(stdout, "\n");
  return out_length;
}

