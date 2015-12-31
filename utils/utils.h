#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <time.h>
#include <vector>

/////////////////////////////////////

#define GETID(b1, b2, b3, b4) (u_int)(((u_char)(b1) << 24) | ((u_char)(b2) << 16) | ( (u_char)(b3) << 8) | (u_char)(b4))

/////////////////////////////////////

const uint default_freq				= 50;
const uint usec_in_sec				= 1000000;
const uint msec_in_sec				= 1000;
const uint usec_in_msec				= 1000;
const uint nsec_in_usec				= 1000;

//////////////////////////////////////

const u_char addr_mpu6050 = 0x68;		/// gyroscope
const u_char addr_hmp5883 = 0x1E;		/// magnetometer
const u_char addr_bmp180 = 0x77;		/// barometer

///////////////////////////////////////

const int freq_gyroAndCompass = 100;
const int period_gyroAndCompass = 1000/freq_gyroAndCompass;

const int freq_baro = 500;
const int period_baro = 1000/freq_baro;

//////////////////////////////////////

std::int64_t get_curtime_usec();
std::int64_t get_curtime_msec();
void _usleep(uint usec);
void _msleep(uint msec);

timespec get_from_ms(u_int ms);

void show_nbytes(const std::vector< u_char >& packet, size_t len);

#endif // UTILS_H
