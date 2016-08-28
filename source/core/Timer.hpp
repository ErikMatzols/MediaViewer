#ifndef TIMER_HPP
#define TIMER_HPP

#include <windows.h>

class Timer {
public:
    Timer();
    ~Timer();

    static void timeBeginPeriod(UINT uPeriod);
    static void timeEndPeriod(UINT uPeriod);

    bool hasPerformanceSupport();
    double getTime();
    void setTime(double t);
    void pause();
    void resume();
    void reset();

protected:
private:
    __int64 mBaseTime;
    __int64 mCurrTime;
    double mCurrTimeSec;
    double mInvFreq;
    bool mPerfFlag;
    bool mPaused;
};

#endif
