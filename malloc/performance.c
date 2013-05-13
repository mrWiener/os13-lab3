/*
 * Please see performance.h for documentation.
 */

#include "performance.h"
#include <stdio.h>

static clock_t begin;   /* Variable to hold number of clock ticks measured by startMeasure. */
static double elapsed = 0;  /* Variable to hold accumulated number of seconds of code execution between start/stop blocks. */
static clock_t elapsed_ticks = 0; /* Variable to hold accumulated number of ticks of code execution between start/stop blocks. */

/*
 * A function to retrieve the number of clock ticks since program start.
 * Will terminate process on error with a value of 1.
 */
clock_t getTicks(void) {
  clock_t ticks; /* Variable to hold the number of clock ticks since program start. */
  
  /* Get the number of clock ticks since program start by calling clock() function. */
  ticks = clock();
  
  /* Check error. */
  if(ticks == -1) {
    /* Error occured. */
    
    /* Exit process with value 1 to indicate error. */
    exit(1);
  }
  
  return ticks; /* Return the number of clock ticks since program start. */
}

void startMeasure(void) {
  /* Simply store the current number of clock ticks since program start into static variable begin. */
  begin = getTicks();
}

double stopMeasure(void) {
  clock_t end; /* Variable to hold current number of clock ticks since program start. */
  double time; /* Variable to hold the number of seconds elapsed since last startMeasure. */
  
  /* Get the number of clock ticks since program start. */
  end = getTicks();
  
  /* Add to accumulated number of ticks variable. */
  elapsed_ticks += (end - begin);
  
  /* Calculate the number of seconds elapsed since last startMeasure call. */
  time = (double)(end - begin) / CLOCKS_PER_SEC;
  
  /* Add it to the static elapsed variable. */
  elapsed += time;
  
  /* Return the time elapsed since last startMeasure. */
  return time;
}

double getMeasuredTime(void) {
  /* Simply return the static variable. */
  return elapsed;
}

clock_t getMeasuredTicks(void) {
  /* Simply return the static variable. */
  return elapsed_ticks;
}
