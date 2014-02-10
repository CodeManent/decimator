#ifndef TIMER_H_
#define TIMER_H_

class Timer{
	unsigned long startTime;
	unsigned long stopTime;
public:
	Timer();
	~Timer();
	void reset();
	void start();
	void stop();
	unsigned long getDuration();
};


#endif

