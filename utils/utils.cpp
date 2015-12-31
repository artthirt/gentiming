#include "utils.h"
#include <sstream>

using namespace std;

int64_t get_curtime_usec()
{
	timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	return (tm.tv_nsec + (tm.tv_sec * 1e+9))/1000;
}

int64_t get_curtime_msec()
{
	int64_t msec = get_curtime_usec();
	return msec / 1000;
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

void show_nbytes(const std::vector<u_char> &packet, size_t len)
{
	stringstream ss;
	for(size_t i = 0; i < std::min(len, len); i++){
		ss << (uint)packet[i] << ", ";
	}
	std::cout << "data[" << ss.str() << "]" << std::endl;
}

timespec get_from_ms(u_int ms)
{
	timespec res;
	res.tv_nsec = (ms * 1000000) % 1000000000;
	res.tv_sec = ms / 1000;
	return res;
}
