#include "utils.h"

using namespace std;

int64_t get_curtime_usec()
{
	timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	return (tm.tv_nsec + (tm.tv_sec * 1e+9))/1000;
}

void _usleep(uint usec)
{
	timespec tm;
	tm.tv_nsec = (usec * nsec_in_usec) % (uint)1e+9;
	tm.tv_sec = usec / usec_in_sec;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &tm, 0);
}

void _msleep(uint msec)
{
	timespec tm;
	tm.tv_nsec = (msec * nsec_in_usec * usec_in_msec) % (uint)1e+9;
	tm.tv_sec = (msec) / msec_in_sec;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &tm, 0);
}

