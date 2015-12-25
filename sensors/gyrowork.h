#ifndef GYROWORK_H
#define GYROWORK_H

#include <stdio.h>
#include <vector>
#include <iostream>

#include "i2cdevice.h"

class control_module;

class gyrowork
{
public:
	gyrowork();

	void init();
	void set_control_module(control_module* cm);
	void close();

	void run();

private:
	I2CDevice m_i2cMpu6050;
	control_module* m_control_module;
	bool m_done;
	/**
	 * @brief get_data
	 * general function for read data
	 */
	void get_data();
};

#endif // GYROWORK_H
