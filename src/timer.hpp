#ifndef TIMER_HPP__
#define TIMER_HPP__

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

namespace SE {  
double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}
}

#endif // TIMER_HPP__
