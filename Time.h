#ifndef TIME_H
#define TIME_H
#include <time.h>

typedef unsigned long Nanosecond;
#define NsPerSecond 1000000000L

namespace Timer{
    Nanosecond GetCurrentTime(){
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * NsPerSecond + ts.tv_nsec;
    }
};

#endif // !TIME_H