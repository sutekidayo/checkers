//Timer.cpp
//Matthew Chandler
//A simple, but precise, cross-platform timer class.
//originaly written for a game I wrote.
//most of the code is heavily based off of other sources

#include "timer.h"
//#if defined WIN32
//timer  modified from NeHe tutorial #21 (http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=21)
//The windows timer uses one of 2 methods. From the site:
//If the program detects a performance counter, the variable performance_timer variable is set to TRUE,
//and all timing is done using the performance counter (alot more accurate than the multimedia timer).
//If a performance counter is not found, performance_timer is set to FALSE and the multimedia timer is used for timing."
//It seems needlessly complex to me, but is so far the most accurate and reliable windows timer I have used.
#include <iostream>
using std::cout;
struct
{
  __int64       frequency;
  double         resolution;
  unsigned long mm_timer_start;
  unsigned long mm_timer_elapsed;
  bool		performance_timer;
  __int64       performance_timer_start;
  __int64       performance_timer_elapsed;
} timer;
void TimerInit(void)								// Initialize Our Timer (Get It Ready) (copied from NeHe tutorial #21 http://nehe.gamedev.net)
{
	memset(&timer, 0, sizeof(timer));					// Clear Our Timer Structure

	// Check To See If A Performance Counter Is Available
	// If One Is Available The Timer Frequency Will Be Updated
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))//Seems to not compile on some machines...
	{
		 //No Performace Counter Available
		timer.performance_timer	= FALSE;				// Set Performance Timer To FALSE
		timer.mm_timer_start	= timeGetTime();			// Use timeGetTime() To Get Current Time
		timer.resolution	= .001f;				// Set Our Timer Resolution To .001f
		timer.frequency		= 1000;					// Set Our Timer Frequency To 1000
		timer.mm_timer_elapsed	= timer.mm_timer_start;			// Set The Elapsed Time To The Current Time
	}
	else
	{
		// Performance Counter Is Available, Use It Instead Of The Multimedia Timer
		// Get The Current Time And Store It In performance_timer_start
		QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
		timer.performance_timer		= TRUE;				// Set Performance Timer To TRUE
		// Calculate The Timer Resolution Using The Timer Frequency
		timer.resolution		= (float) (((double)1.0f)/((double)timer.frequency));
		// Set The Elapsed Time To The Current Time
		timer.performance_timer_elapsed	= timer.performance_timer_start;
	}
}

double TimerGetTime(void)								// Get Time In Milliseconds (copied from NeHe tutorial #21 http://nehe.gamedev.net)
{
	__int64 time;								// time Will Hold A 64 Bit Integer

	if (timer.performance_timer)						// Are We Using The Performance Timer?
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &time);		// Grab The Current Performance Time
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return ( (float) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
	}
	else
	{
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return( (float) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
	}
}
/*#else // UNIX or other system
//modified from Dr. Lawlor lecture notes http://www.cs.uaf.edu/2008/fall/cs301/lecture/10_24_performance.html
double TimerGetTime(void)//return ms
{
	struct timeval tv;
	gettimeofday(&tv,0);
	return (tv.tv_usec*1.0e-6+(double)tv.tv_sec)*1.0e+3;
}
#endif*/
//quick & dirty pause routine
void timer_pause(long ms)
{
	TIMER pause;
	pause.start=TimerGetTime();
	while((pause.elapsed=TimerGetTime()-pause.start)<ms);
}
