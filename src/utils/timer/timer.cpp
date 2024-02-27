#pragma once

#include "timer.h"

void MyTimer::ResetTime() {
    startTime = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
}

double MyTimer::GetDurationTime() {
    milliseconds now = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return milliseconds.count() / 1000.0;
}

double MyTimer::GetCurrentTimeinSce() {
    milliseconds now = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return now.count() / 1000.0;
}

double MyTimer::GetCurrentTime() {
    milliseconds now = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
    );
    return now.count();
}

milliseconds MyTimer::startTime = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
);