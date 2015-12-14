#ifndef GPIOWORK_H
#define GPIOWORK_H

#include <map>
#include <string>
#include <mutex>
#include <boost/thread.hpp>

#include "utils.h"

class control_module;

namespace gpio{

class gpiopin{
public:
	enum CASE{
		ONE = 0,
		ZERO
	};

	gpiopin();
	gpiopin(const gpiopin& cp);
	gpiopin(uint _impulse_usec, uint _period_usec, int _pin);

	~gpiopin();

	gpiopin& operator= (const gpiopin& cp);

	void run();

	boost::thread thread;

	bool done;
	uint impulse_usec;
	uint period_usec;
	int pin;
	CASE cur_case;
	long long last_time;

	int64_t counter;

	uint delay() const;
	uint delay_zero() const;

	void set_data(uint _impulse_usec, uint _period_usec);

	uint getPeriod() const;
	void setPeriod(const uint &value);

	void swap_case();

private:
	void set_one();
	void set_zero();
};

class gpiowork
{
public:
	gpiowork();
	/**
	 * @brief open_pin
	 * set timouts for pin in milliseconds
	 * @param pin
	 * @param impulse
	 * @param meandr
	 * @return
	 */
	bool open_pin(int pin, float impulse, float meandr);
	void close();
	void set_control_module(control_module* cm);

	void run();
private:
	std::map< int, gpiopin > m_mappin;
	bool m_done;
	control_module *m_control_module;

	void check_pins();
};

}

#endif // GPIOWORK_H
