/*****************************************************************************
 *
 * microwork_inline_test.c
 *
 * Example of using the inline work loops and calibration method to 
 * model workload latency. In this case, we collect data about the 
 * accuracy of at loop by comparing target duration with actual duration.
 *
 * Author: Whit Schonbein
 * Email: whit.schonbein@gmail.com
 * Date: 2016
 * 
 * Written under auspices of Dorian Arnold and the University of New Mexico
 *
 *****************************************************************************/

#include "microwork_inline_test.h"

void usage(char **argv) {
  printf("\n################################################################\n");
  printf("Usage:\n");
  printf("  %s -c <cycles> -t <trials> -d <nsecs> -n <tests> -r <rest_mode> -v\n", argv[0]);
  printf("\nWhere:\n");
  printf("  -c <cycles> : number of cycles per calibration trial (required but ignored if work method is WORK_MXM)\n");
  printf("  -d <nsecs>  : duration of each test (required)\n");
  printf("  -n <tests>  : number of tests to perform (required)\n");
  printf("  -t <trials> : number of calibration trials (required)\n");
  printf("  -r <int>    : rest mode for between trials and tests (required)\n");
  printf("                  0 = sleep(1)\n");
  printf("                  1 = write to /dev/null\n");
  printf("  -v          : verbose (optional)\n");
  printf("################################################################");
  printf("\n");
  exit(-1);
}

/* 
 * Processes command line argument
 */
int process_args( int argc, char **argv, optargs_t *opts) {
  int c;
  extern char *optarg;
  extern int optind, optopt;
  
  /* flags */
  int c_flag = 0;
  int d_flag = 0;
  int n_flag = 0;
  int r_flag = 0;
  int t_flag = 0;
  
  /* set options defaults */
  set_default_options(opts);

  while ((c = getopt(argc, argv, "c:d:n:r:t:v")) != -1) {
    switch(c) 
    {  
      case 'c': /* number of cycles per trial */
        c_flag = 1;
        opts->cycles_per_trial = strtoull(optarg,NULL,10);
        break;
      case 'd': /* duration of tests */
        d_flag = 1;
        opts->target_nsec = strtoull(optarg,NULL,10);
        break;
      case 'n': /* number of tests */
        n_flag = 1;
        opts->num_tests = atoi(optarg);
        break;
      case 'r': /* rest mode */
        r_flag = 1;
        opts->rest_mode = atoi(optarg);
        break;
      case 't': /* number of calibration trials */
        t_flag = 1;
        opts->num_trials = atoi(optarg);
        break;
      case 'v': /* verbose */
        opts->verbose = 1;
        break;
      case '?':
        fprintf(stderr, "Unkown option -%c\n", optopt);
        usage(argv);
        break;
      default:
        usage(argv);
        break;
    }
  }

  if (!c_flag) {
    fprintf(stderr, "\n-c option required\n");
    usage(argv);
  }

  if (!d_flag) {
    fprintf(stderr, "\n-d option required\n");
    usage(argv);
  }

  if (!n_flag) {
    fprintf(stderr, "\n-n option required\n");
    usage(argv);
  }

  if (!r_flag) {
    fprintf(stderr, "\n-r option required\n");
    usage(argv);
  }

  if (!t_flag) {
    fprintf(stderr, "\n-t option required\n");
    usage(argv);
  }

  return 0;
}

void set_default_options( optargs_t *options ) {
  options->cycles_per_trial = 0;
  options->num_trials = 0;
  options->rest_mode = 0;
  options->num_tests = 0;
  options->target_nsec = 0;
  options->verbose = 0;
} 

/*
 * Mr. Main
 */
int main( int argc, char ** argv ) {

  int t;
  c_results_t c_results;  /* results of calibration */
  optargs_t options;      /* options */

  /* process command line */
  if (process_args( argc, argv, &options) != 0 ) {
    fprintf(stderr, "%s:%d ERROR -- failure parsing command line.\n", __FILE__,__LINE__);
    return -1;
  }

  /* calibrate the work loop */
  #if defined( WORK_NULL ) || defined( WORK_MXM ) || defined( WORK_ASM_NOP ) || defined( WORK_ASM_MUL ) || defined( WORK_ASM_FADD ) || defined( WORK_ASM_FMUL )
    if (options.verbose) printf("Calibrating:\n");
    calibrate(options.num_trials, options.cycles_per_trial, options.rest_mode, options.verbose, &c_results);
  #else
    fprintf(stderr, "%s:%d: ERROR -- unkown work type.\n", __FILE__, __LINE__);
    return -1;
  #endif

  /* calculate the number of loop iterations (MXM) or cycles (ASM) required */
  uint64_t loop_num = calc_loop_num(options.target_nsec,&c_results);

  /* print out options and results of calibration */
  fprintf(stdout,"#############################################\n");
  fprintf(stdout,"# calibration trials: %d\n", options.num_trials);
  #if !defined( WORK_MXM )
    fprintf(stdout,"# cycles per trial  : %lld\n", options.cycles_per_trial);
  #endif  
  fprintf(stdout,"# rest_mode         : %d\n", options.rest_mode);
  fprintf(stdout,"# num_tests         : %d\n", options.num_tests);
  fprintf(stdout,"# target_nsec       : %lld\n", options.target_nsec);
  fprintf(stdout,"# verbose           : %d\n", options.verbose);
  fprintf(stdout,"#############################################\n");
  fprintf(stdout,"# Average           : %f\n", c_results.average);
  fprintf(stdout,"# Std dev           : %f\n", c_results.std_dev);
  fprintf(stdout,"# Min               : %lld\n", c_results.min);
  fprintf(stdout,"# Max               : %lld\n", c_results.max);
  fprintf(stdout,"# calibration cycles: %lld\n", c_results.calibration_cycles);
  fprintf(stdout,"# target            : %lld\n", c_results.target_nsec);
  fprintf(stdout,"# loop_num          : %lld\n", c_results.loop_num);
  fprintf(stdout,"#############################################\n");
  fprintf(stdout,"# target_nsec # trial 1 nsec # ... # trial t nsec # ratio of average difference to target_nsec #\n");

  /* perform the tests */
  struct timespec start,end;
  /* mach/osx uses a different method to get times */
  #if defined(__MACH__)
    clock_serv_t cclock;
    mach_timespec_t mts_start, mts_end;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  #endif
  uint64_t *results = malloc(options.num_tests * sizeof(*results));

  fprintf(stdout,"%lld\t", options.target_nsec);
  for (t=0;t<options.num_tests;t++) {
    
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
      fprintf(stderr, "%s:%d: ERROR -- unkown work type.\n", __FILE__, __LINE__);
      #if defined(__MACH__)
        mach_port_deallocate(mach_task_self(), cclock);
      #endif
      return -1;
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
    
    results[t] = timespec_sub(&start, &end);
    fprintf(stdout,"%lld\t", results[t]);
    fflush(stdout);
    
    /* rest */
    switch (options.rest_mode) {
      case REST_SLEEP:
        sleep(1);
        break;
      case REST_DEV_NULL:
        rest_dev_null(SLEEP_CYCLES);
        break;
      default:
        fprintf(stderr, "%s:%d: ERROR -- unknown rest mode.\n", __FILE__, __LINE__);
        sleep(1);
    }
  }
 
  /* get average ratio */
  uint64_t sum = 0.;
  for (t=0;t<options.num_trials;t++) {
    sum += abs(options.target_nsec - results[t]);
  }
  double avg_err = sum/(double)options.num_trials;
  double avg_ratio = avg_err/(double)options.target_nsec;

  fprintf(stdout,"%f\n", avg_ratio);
   
  #if defined(__MACH__)
    mach_port_deallocate(mach_task_self(), cclock);
  #endif

  free(results);

  return 0;
}

