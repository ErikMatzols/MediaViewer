#include "Timer.hpp"

Timer::Timer()
{
    mCurrTime = 0;
    mBaseTime = 0;
    mPerfFlag = true;
    mPaused = false;
    mCurrTimeSec = 0;
    __int64 mPerfFreq;

    if (QueryPerformanceFrequency((LARGE_INTEGER*)&mPerfFreq)) {
        QueryPerformanceCounter((LARGE_INTEGER*)&mBaseTime);
        mInvFreq = 1.0 / mPerfFreq;
    } else {
        mPerfFlag = false;
    }
}

Timer::~Timer()
{
}

bool Timer::hasPerformanceSupport()
{
    return mPerfFlag;
}

double Timer::getTime()
{
    if (!mPaused) {
        QueryPerformanceCounter((LARGE_INTEGER*)&mCurrTime);
        mCurrTimeSec = ((mCurrTime - mBaseTime) * mInvFreq);
    }

    return mCurrTimeSec;
}

void Timer::setTime(double t)
{
    QueryPerformanceCounter((LARGE_INTEGER*)&mCurrTime);
    mBaseTime = mCurrTime - (__int64)(t / mInvFreq);
    mCurrTimeSec = (mCurrTime - mBaseTime) * mInvFreq;
}

void Timer::pause()
{
    if (!mPaused) {
        QueryPerformanceCounter((LARGE_INTEGER*)&mCurrTime);
        mCurrTimeSec = ((mCurrTime - mBaseTime) * mInvFreq);
        mPaused = true;
    }
}

void Timer::resume()
{
    if (mPaused) {
        __int64 resumeTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&resumeTime);
        mBaseTime = resumeTime - (mCurrTime - mBaseTime);
        mPaused = false;
    }
}

void Timer::reset()
{
    mCurrTime = 0;
    mBaseTime = 0;
    mPaused = false;
    mPerfFlag = true;
    mCurrTimeSec = 0;

    __int64 mPerfFreq;

    if (QueryPerformanceFrequency((LARGE_INTEGER*)&mPerfFreq)) {
        QueryPerformanceCounter((LARGE_INTEGER*)&mBaseTime);
        mInvFreq = 1.0 / mPerfFreq;
    } else {
        mPerfFlag = false;
    }
}

void Timer::timeBeginPeriod(UINT uPeriod)
{
    ::timeBeginPeriod(uPeriod);
}

void Timer::timeEndPeriod(UINT uPeriod)
{
    ::timeEndPeriod(uPeriod);
}
