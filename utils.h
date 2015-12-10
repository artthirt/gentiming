#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <time.h>

/////////////////////////////////////

const uint default_freq				= 50;
const uint usec_in_sec				= 1000000;
const uint msec_in_sec				= 1000;
const uint usec_in_msec				= 1000;
const uint nsec_in_usec				= 1000;

//////////////////////////////////////

std::int64_t get_curtime_usec();
void _usleep(uint usec);
void _msleep(uint msec);

#endif // UTILS_H
