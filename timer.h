//timer.h
//Matthew Chandler
// Declarations for a simple, but precise, cross-platform timer class


#ifndef __TIMER_H__
#define __TIMER_H__
//Windows version of declarations
//#if defined WIN32
#include <windows.h>// Header File For Windows libraries
#pragma comment( lib, "winmm.lib" )		//Search For winmm.lib While Linking
double TimerGetTime(void);//return ms
void TimerInit(void);
//I worte a simple class wrapper for the timer functions, which were modified from http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=21
class TIMER{
public:
	TIMER()
	{
		TimerInit();
	}
	double start;
	double elapsed;

};
void timer_pause(long ms); //pause for ms milliseconds*/
/*#else // UNIX or other system
//modified from Dr. Lawlor lecture notes http://www.cs.uaf.edu/2008/fall/cs301/lecture/10_24_performance.html
#include <sys/time.h>
class TIMER{
public:
	double start;
	double elapsed;
};
double TimerGetTime(void);//return ms
#endif
//quick & dirty pause routine
void timer_pause(long ms); //pause for ms milliseconds*/
#endif
















