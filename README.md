# microwork
###############################################################################

Microwork loops for modeling latency in synthetic benchmarks.

Whit Schonbein
University of New Mexico
whit.schonbein@gmail.com
2016,2017

This code is provided without warranty; use at your own risk.

###############################################################################

###############################################################################

This is a cleaned-up version of a subset of the code I used for my 
MS Computer Science project.

Problem: When developing benchmarks we sometimes want the benchmark to 
do a busy wait (i.e., do some `work' for a some amount of time). Often this 
is accomplished through in-cache matrix multiplication, divided into two 
phases: (1) during the calibration phase, the duration of the matrix operation 
is measured over multiple trials, and an average duration calculated; (2) when 
the busy wait is needed, the number of matrix multiplications required to use 
up a certain amount of time is calculated and the operations performed.

We noticed the difference between the requested work duration and the 
duration actually delivered could be significant, e.g., exceeding 50%. 
This error can be reduced by strategies such as increasing the number of 
calibration trials, reducing OS noise (e.g., by running on a lightweight 
kernel), and by not using sleep(n) between calibration trials. However, 
we nonetheless wanted to see if there existed a more accurate busy 
wait method, one more tolerant of OS noise, and which might be able to 
deliever fine-grained durations (on the order of thousands of nanoseconds).

Solution: A collection of work loops utilizing the time stamp counter 
(available on most modern commodity cpus) to control exiting the loop, and 
a variety of basic operations (NOP, MUL, floating point MUL) to do 
nonsense work. Using these loops, we've observered reductions in error 
between requested and actual duration by multiple orders of magnitude in 
comparison to a naive matrix multiplication approach, when run on 
standard commodity hardware with the usual OS suspects (Linux, OSX).

There are many caveats and qualifications I won't go into.

###############################################################################

###############################################################################

The microwork loops require a time stamp counter with certain features; 
consult proc/cpuinfo:

The following flags are relevant (tsc = time stamp counter):
1.  'tsc' indicates the presence of the tsc functionality (non-sycnchronizing)
2.  'rdtscp' indicates the synchronizing version is available
3.  'constant_tsc' indicates whether the frequency of the tsc is constant 
      across dynamic frequency voltage scaling; it is the 'architecture' 
      timer. THE NEW LOOPS CANNOT BE USED IF THIS FLAG IS NOT PRESENT.
4.  'nonstop_tsc' indicates whether the counter ever stops.
5.  'tsc_deadline_timer' - not sure what this means.
6.  'tsc_adjust' - not sure what this means.

The combination of constant_tsc and nonstop_tsc is sometimes referred to 
as an 'invariant' time stamp counter. This is desireable.

On Linux, the calibration phase uses the system clock (CLOCK_MONOTONIC).

To see available clock sources:
  cat /sys/devices/system/clocksource/clocksource0/available_clocksource

To see which one is being used by kernel:
  cat /sys/devices/system/clocksource/clocksource0/current_clocksource

###############################################################################

###############################################################################

The microwork loops are intended to be inlined into the code at the point 
work is to be done. Before that point, the loop must be calibrated. 
See microwork_inline_test.c for an example of a program that (i) uses 
the provided calibration method, and (ii) inlines microwork loops that 
use the results of calibration. 

###############################################################################


