#include "gpiowork.h"

#include <iostream>
#include <time.h>

#include "gpiorw.h"
#include "struct_controls.h"
#include "control_module.h"

#include <boost/thread.hpp>

using namespace std;
using namespace gpio;
using namespace boost;
using namespace sc;

//////////////////////////////

const float min_impulse = 1.;
const float min_angle = -180.;

const float max_impulse = 2.;
const float max_angle = 180.;

/**
 * @brief get_impulse
 * generate impulse for servo sg90
 * @param angle
 * @return - impulse in milliseconds
 */
float get_impulse(float angle)
{
	float delta = max_impulse - min_impulse;
	float delta_angle = (max_angle - min_angle);
	float res = angle - min_angle;
	res /= delta_angle;
	res = min_impulse + res * delta;
	return res;
}

//////////////////////////////

gpiopin::gpiopin()
	: impulse_usec(0)
	, period_usec(0)
	, pin(-1)
	, done(false)
	, exited(false)
	, cur_case(ONE)
	, counter(0)
	, start_time(0)
	, last_time(0)
	, is_init(false)
{

}

gpiopin::gpiopin(const gpiopin &cp)
	: is_init(false)
{
	impulse_usec = cp.impulse_usec;
	period_usec = cp.period_usec;
	pin = cp.pin;
	cur_case = cp.cur_case;
	start_time = cp.start_time;
	last_time = cp.last_time;
	counter = cp.counter;
	exited = cp.exited;
	done = cp.done;
}

gpiopin::gpiopin(uint _impulse_usec, uint _period_usec, int _pin)
	: done(false)
	, exited(false)
	, cur_case(ONE)
	, counter(0)
	, start_time(0)
	, last_time(0)
	, is_init(false)
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

void gpio::gpiopin::run()
{
	int64_t tm1, tm2;
	int id;
	exited = false;
	done = false;
	while(!done){
		if(is_init){
			tm1 = get_curtime_usec();
			switch (cur_case) {
				case ONE:
					set_one();
					id = 1;
					break;
				case ZERO:
				default:
					set_zero();
					id = 0;
					break;
			}
			swap_case();
			tm2 = get_curtime_usec();
			cout << "pin[" << pin << "] id[" << id << "]: " << tm2 - tm1 << " usec    counter[" << counter << "]\n";
			counter++;
		}else{
			_msleep(100);
		}
	}
	exited = true;
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

void gpiopin::set_one()
{
	/// 1 >> pin/value
	gpio::gpiorw::instance().write_to_pin(pin, true);
	_usleep(impulse_usec);
}

void gpiopin::set_zero()
{
	/// 0 >> pin/value
	gpio::gpiorw::instance().write_to_pin(pin, false);
	_usleep(delay_zero());
}

//////////////////////////////////////

gpiowork::gpiowork()
	: m_done(false)
{

}

bool gpiowork::open_pin(int pin, float impulse, float meandr, bool init)
{
	if(m_mappin.find(pin) == m_mappin.end()){
		m_mappin[pin] = gpiopin(impulse * usec_in_msec, meandr * usec_in_msec, pin);
		m_mappin[pin].is_init = init;
		m_mappin[pin].thread = thread(boost::bind(&gpiopin::run, &m_mappin[pin]));

	}else{
		m_mappin[pin].set_data(impulse * usec_in_msec, meandr * usec_in_msec);
	}

	return true;
}

void gpiowork::close(int pin)
{
	if(m_mappin.find(pin) == m_mappin.end()){
		return;
	}
	m_mappin[pin].done = true;
}

void gpiowork::close()
{
	m_done = true;
	for(auto it = m_mappin.begin(); it != m_mappin.end(); it++){
		it->second.done = true;
	}
}

void gpiowork::run()
{
	while(!m_done){
		control_pins();
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

void gpiowork::control_pins()
{
	for(auto it = m_mappin.begin(); it != m_mappin.end(); it++){
		gpiopin& pin = it->second;
		if(pin.exited){
			it = m_mappin.erase(it);
		}else{
			if(pin.is_init && get_curtime_msec() > pin.last_time)
				pin.done = true;
		}
	}
	/// empty
}

void gpiowork::handler_signal(const StructControls &sc)
{
	int pin = sc.servo_ctrl.pin;
	float imp = get_impulse(sc.servo_ctrl.angle);
	float meandr = sc.servo_ctrl.freq_meandr;

	if(m_mappin.find(pin) == m_mappin.end()){
		open_pin(pin, imp, meandr, false);
	}

	gpiopin &gpin = m_mappin[pin];

	gpin.set_data(imp, meandr);
	gpin.is_init = false;
	gpin.start_time = get_curtime_msec();
	gpin.last_time = gpin.start_time + sc.servo_ctrl.timework_ms;
	gpin.is_init = true;
}
