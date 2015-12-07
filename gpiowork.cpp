#include "gpiowork.h"

#include <iostream>
#include <time.h>

#include "gpiorw.h"

#include <boost/thread.hpp>

using namespace std;
using namespace gpio;
using namespace boost;

/////////////////////////////////////

const uint default_freq				= 50;
const uint usec_in_sec				= 1000000;
const uint msec_in_sec				= 1000;
const uint usec_in_msec				= 1000;
const uint nsec_in_usec				= 1000;

//////////////////////////////////////

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

//////////////////////////////////////

gpiopin::gpiopin()
	: impulse_usec(0)
	, period_usec(0)
	, pin(-1)
	, done(false)
	, cur_case(ONE)
	, counter(0)
{

}

gpiopin::gpiopin(const gpiopin &cp)
{
	impulse_usec = cp.impulse_usec;
	period_usec = cp.period_usec;
	pin = cp.pin;
	cur_case = cp.cur_case;
	last_time = cp.last_time;
	counter = cp.counter;
}

gpiopin::gpiopin(uint _impulse_usec, uint _period_usec, int _pin)
	: done(false)
	, cur_case(ONE)
	, counter(0)
{
	impulse_usec = _impulse_usec;
	period_usec = _period_usec;

	if(period_usec < impulse_usec){
		cout << "warning. period less than impulse. recalc impulse\n";
		impulse_usec = period_usec;
	}

	pin = _pin;

	if(!gpiorw::instance().open_pin(pin)){
		cout << "pin " << pin << " not opened\n";
	}
}

gpiopin::~gpiopin()
{

}

gpiopin &gpiopin::operator=(const gpiopin &cp)
{
	impulse_usec = cp.impulse_usec;
	period_usec = cp.period_usec;
	pin = cp.pin;
	cur_case = cp.cur_case;
	last_time = cp.last_time;
	counter = cp.counter;
	return *this;
}

void gpio::gpiopin::operator ()()
{
	int64_t tm1, tm2;
	int id;
	while(!done){
		tm1 = get_curtime_usec();
		switch (cur_case) {
			case ONE:
				sleep_one();
				id = 1;
				break;
			case ZERO:
			default:
				sleep_zero();
				id = 0;
				break;
		}
		swap_case();
		tm2 = get_curtime_usec();
		cout << "pin[" << pin << "] id[" << id << "]: " << tm2 - tm1 << " usec    counter[" << counter << "]\n";
		counter++;
	}
}

uint gpiopin::getPeriod() const
{
	return period_usec;
}

void gpiopin::setPeriod(const uint &value)
{
	period_usec = value;
}

void gpiopin::swap_case()
{
	switch (cur_case) {
		case ONE:
			cur_case = ZERO;
			break;
		case ZERO:
		default:
			cur_case = ONE;
			break;
	}
}

uint gpiopin::delay_zero() const
{
	return period_usec - impulse_usec;
}

void gpiopin::set_data(uint _impulse_usec, uint _period_usec)
{
	impulse_usec = _impulse_usec;
	period_usec = _period_usec;

	if(period_usec < impulse_usec){
		cout << "warning. period less than impulse. recalc impulse\n";
		impulse_usec = period_usec;
	}

}

uint gpiopin::delay() const
{
	switch (cur_case) {
		case ONE:
			return impulse_usec;
		case ZERO:
		default:
			return delay_zero();
	}
}

void gpiopin::sleep_one()
{
	_usleep(impulse_usec);
}

void gpiopin::sleep_zero()
{
	_usleep(delay_zero());
}

//////////////////////////////////////

gpiowork::gpiowork()
	: m_done(false)
{

}

bool gpiowork::open_pin(int pin, float impulse, float meandr)
{
	if(m_mappin.find(pin) == m_mappin.end()){
		m_mappin[pin] = gpiopin(impulse * usec_in_msec, meandr * usec_in_msec, pin);

		m_mappin[pin].thread = thread(m_mappin[pin]);

	}else{
		m_mappin[pin].set_data(impulse * usec_in_msec, meandr * usec_in_msec);
	}

	return true;
}

void gpiowork::close()
{
	m_done = true;
	for(auto it = m_mappin.begin(); it != m_mappin.end(); it++){
		it->second.done = true;
	}
}

void gpiowork::operator()()
{
	while(!m_done){
		_msleep(100);
	}
	check_pins();
}

void gpiowork::check_pins()
{
	for(auto it = m_mappin.begin(); it != m_mappin.end(); it++){
		gpiopin& pin = it->second;
		pin.thread.join();
	}
}
