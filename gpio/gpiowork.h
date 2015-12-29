#ifndef GPIOWORK_H
#define GPIOWORK_H

#include <map>
#include <string>
#include <mutex>
#include <atomic>
#include <boost/thread.hpp>

#include "utils.h"
#include "struct_controls.h"

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

	uint delay() const;
	uint delay_zero() const;

	void set_data(uint _impulse_usec, uint _period_usec);

	uint getPeriod() const;
	void setPeriod(const uint &value);

	void swap_case();

public:
	boost::thread thread;

public:

	std::atomic<bool> is_init;		/// for wait after open_pin any operations
	bool done;						/// if true run() break
	bool exited;					/// true when exit from run()
	uint impulse_usec;				/// 1 delay
	uint period_usec;				/// all delay
	int pin;						/// number pin
	CASE cur_case;
	long long last_time;
	long long start_time;

	int64_t counter;

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
	 * @param init - true for start immediatly
	 * @return
	 */
	bool open_pin(int pin, float impulse, float meandr, bool init = true);
	void close(int pin);
	void close();

	void run();

	void handler_signal(const sc::StructControls& sc);
private:
	std::map< int, gpiopin > m_mappin;
	bool m_done;

	void check_pins();
	void control_pins();
};

}

#endif // GPIOWORK_H
