#include <time.h> /* Used for the CLOCKS_PER_SEC constant. */
#include <stdlib.h>

/*
 * To measure the elapsed time for code execution, this function should be called right before the code
 * to measure. It will initialize an internal static variable to the current number of ticks since program start.
 */
void startMeasure(void);

/*
 * To measure the elapsed time for code execution, this function should be called right after the code
 * to measure. This will compute the elapsed time by using the static variable initialized by startMeasure above.
 * This function returns the number of seconds elapsed since the call to startMeasure, and will then add this value
 * to an internal static variable which holds the total elapsed time. This value can be retrieved by calling getMeasuredTime.
 */
double stopMeasure(void);

/*
 * This will return the total elapsed time accumulated by start/stop calls. The returned value will be in seconds.
 */
double getMeasuredTime(void);

/*
 * This will return the total elapsed ticks accumulated by start/stop calls. The returned value will be in seconds.
 */
clock_t getMeasuredTicks(void);
