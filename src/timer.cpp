#include "timer.hpp"
#include <ctime>
#include <inttypes.h>
#include <stdexcept>



//Code adapted from: code-factor.blogspot.gr/2009/11/monotonic-timers.html
#ifdef LINUX
uint64_t mili_count(){
	struct timespec t;
	int ret;
	ret = clock_gettime(CLOCK_MONOTONIC, &t);
	if(ret != 0)
		throw std::runtime_error("Timer nano_count: Couldn't read the system timer.");
	
	return t.tv_sec * 1000 + t.tv_nsec/1000000;
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

