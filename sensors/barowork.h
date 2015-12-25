#ifndef BAROWORK_H
#define BAROWORK_H

#include <iostream>
#include "i2cdevice.h"

class control_module;

class barowork
{
public:
	barowork();

	void init();
	void set_control_module(control_module* cm);
	void close();

	void run();
private:
	I2CDevice m_i2cBmp180;
	control_module* m_control_module;
	bool m_done;

	enum BAROSTATE{
		NULLSTATE		= 0,
		SETTEMP			= 1,
		GETTEMP			= 2,
		GETPRESSURE		= 3,
		SETPRESSURE		= 4
	};
	BAROSTATE m_baroState;
	int m_baro_interval;
	int m_baro_time_set;

	void write_baro_get_pressure();
	void read_baro_get_pressure();

	void write_baro_get_temp();
	void read_baro_get_temp();
	/**
	 * @brief get_data
	 * general function for read data
	 */
	void get_data();
};

#endif // BAROWORK_H
