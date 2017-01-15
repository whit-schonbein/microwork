/*****************************************************************************
 *
 * microwork_inline_test.h
 *
 * Author: Whit Schonbein
 * Email: whit.schonbein@gmail.com
 * Date: 2016
 * 
 * Written under auspices of Dorian Arnold and the University of New Mexico
 *
 *****************************************************************************/

#if !defined( __MICROWORK_INLINE_TEST_H_ )
#define __MICROWORK_INLINE_TEST_H

#include <unistd.h>   /* for getopt */

/* if running on OSX/Mach kernel, requires different clock */
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "microwork_inline.h"

/* runtime options */
typedef struct optargs_s {
  uint64_t cycles_per_trial;  /* number of cycles to peform in each trial */
  int num_trials;             /* number of calibration trials */
  int rest_mode;              /* rest mode for between trials and tests */
  int num_tests;              /* number of tests */ 
  uint64_t target_nsec;       /* desired duration of work */
  int verbose;                /* verbose */
} optargs_t;

/* set default runtime options */
void set_default_options( optargs_t *options );

/* print usage */
void usage();

/* process command line */
int process_args( int argc, char **argv, optargs_t *opts );

#endif /* define __MICROWORK_INLINE_TEST_H_ */
