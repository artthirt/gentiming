#ifndef COMPASSWORK_H
#define COMPASSWORK_H

#include <iostream>
#include "i2cdevice.h"

class control_module;

class compasswork
{
public:
	compasswork();

	void init();
	void get_data();
	void set_control_module(control_module* cm);
	void close();

	void run();

private:
	I2CDevice m_i2cHmc5883;
	control_module* m_control_module;
	bool m_done;
	u_char m_regA_hmc5883;
};

#endif // COMPASSWORK_H
