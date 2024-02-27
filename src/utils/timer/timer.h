#pragma once
#include <chrono>
#include <iostream>
#include <time.h>
#include <ctime>

using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


class MyTimer {
public:
	static milliseconds startTime;
	static void ResetTime();
	static double GetDurationTime();
	static double GetCurrentTimeinSce();
    static double GetCurrentTime();
};
