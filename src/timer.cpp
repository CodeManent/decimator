#include "timer.hpp"
#include <stdexcept>



//Code adapted from: code-factor.blogspot.gr/2009/11/monotonic-timers.html
#ifdef LINUX

#include <ctime>
#include <inttypes.h>

uint64_t mili_count(){
	struct timespec t;
	int ret;
	ret = clock_gettime(CLOCK_MONOTONIC, &t);
	if(ret != 0)
		throw std::runtime_error("Timer mili_count: Couldn't read the system timer.");
	
	return t.tv_sec * 1000 + t.tv_nsec/1000000;
}

#endif
#ifdef windows
#include <windows.h>

typedef unsigned __int64 u64;
typedef unsigned __int32 u32;


u64 mili_count()
{
    static double scale_factor;

    static u32 hi = 0;
    static u32 lo = 0;

    LARGE_INTEGER count;
    BOOL ret = QueryPerformanceCounter(&count);
    if(ret == 0)
		throw std::runtime_error("Timer mili_count: QueryPerformanceCounter");

    if(scale_factor == 0.0)
    {
        LARGE_INTEGER frequency;
        BOOL ret = QueryPerformanceFrequency(&frequency);
        if(ret == 0)
			throw std::runtime_error("Timer mili_count: QueryPerformanceFrequency");
        scale_factor = (1000000000.0 / frequency.QuadPart);
    }

#ifdef FACTOR_64
    hi = count.HighPart;
#else
    /* On VirtualBox, QueryPerformanceCounter does not increment
    the high part every time the low part overflows.  Workaround. */
    if(lo > count.LowPart)
        hi++;
#endif
    lo = count.LowPart;

    return (u64)((((u64)hi << 32) | (u64)lo) * scale_factor)/1000000;
}

#endif


#ifdef OSX

#include <mach/mach_time.h>

u64 mili_count()
{
    u64 time = mach_absolute_time();

    static u64 scaling_factor = 0;
    if(!scaling_factor)
    {
        mach_timebase_info_data_t info;
        kern_return_t ret = mach_timebase_info(&info);
        if(ret != 0)
            fatal_error("mach_timebase_info failed",ret);
        scaling_factor = info.numer/info.denom;
    }

    return (time * scaling_factor) /1000000;
}
#endif



Timer::Timer()
:startTime(0)
,stopTime(0)
{
}

Timer::~Timer(){

}

void Timer::reset(){
	startTime = 0;
	stopTime = 0;
}


void Timer::start(){
	if(startTime)
		throw std::runtime_error("Timer::start: Timer is already running.");
	
	startTime = mili_count();
}

void Timer::stop(){
	if(!startTime)
		throw std::runtime_error("Timer::stop: Timer hasn't started counting.");
	
	stopTime = mili_count();
}

unsigned long Timer::getDuration(){
	if(!startTime)
		throw std::runtime_error("Timer::getDuration: Timer hasn't started.");
	if(!stopTime)
		throw std::runtime_error("Timer::getDuration: Timer is still running.");
	
	return stopTime - startTime;
}

