#include "barowork.h"

#include <vector>

#include "utils.h"
#include "vector3_.h"
#include "struct_controls.h"
#include "control_module.h"

const int default_interval_ms_barometer_read = 30;
const u_char oss_bmp180 = 2;

namespace barotimeset{
	const int temp = 5;
	const int pressure_oss0 = 5;
	const int pressure_oss1 = 8;
	const int pressure_oss2 = 14;
	const int pressure_oss3 = 26;
	int pressure_ossN = pressure_oss0;
}

barowork::barowork()
	: m_done(false)
	, m_control_module(0)
	, m_baroState(NULLSTATE)
	, m_baro_interval(period_baro)
	, m_baro_time_set(0)
{

}

void barowork::init()
{
	if(!m_i2cBmp180.open(addr_bmp180)){
		std::cout << "bmp180: device not open\n";
		return;
	}
	std::cout << "bmp180: device opened\n";

	switch (oss_bmp180) {
		case 0:
		default:
			barotimeset::pressure_ossN = barotimeset::pressure_oss0;
			break;
		case 1:
			barotimeset::pressure_ossN = barotimeset::pressure_oss1;
			break;
		case 2:
			barotimeset::pressure_ossN = barotimeset::pressure_oss2;
			break;
		case 3:
			barotimeset::pressure_ossN = barotimeset::pressure_oss3;
			break;
	}
	m_baro_interval = barotimeset::pressure_ossN;

	m_baroState = SETTEMP;
	m_baro_time_set = get_curtime_msec();
}

void barowork::get_data()
{
	if(!m_i2cBmp180.is_opened())
		return;

	int64_t elapsed = get_curtime_msec();

	if(elapsed > m_baro_time_set + m_baro_interval){
		switch (m_baroState) {
			case NULLSTATE:
			case SETTEMP:
				write_baro_get_temp();
				m_baroState = GETTEMP;
				m_baro_interval = barotimeset::temp;
				break;
			case GETTEMP:
				read_baro_get_temp();
				m_baroState = SETPRESSURE;
				m_baro_interval = 0;
			case SETPRESSURE:
				write_baro_get_pressure();
				m_baroState = GETPRESSURE;
				m_baro_interval = barotimeset::pressure_ossN;
				break;
			case GETPRESSURE:
				read_baro_get_pressure();
				m_baroState = SETTEMP;
				m_baro_interval = 0;
				break;
			default:
				break;
		}
		//qDebug() << m_baroState << m_baro_interval;
		m_baro_time_set = elapsed;
	}
}

void barowork::write_baro_get_temp()
{
	u_char addr_reg = 0xf4;
	u_char val_reg = 0x2e;
	m_i2cBmp180.write(addr_reg, &val_reg, 1);

}

void barowork::read_baro_get_temp()
{
	u_char data[2];
	u_char addr_reg = 0xf6;
	m_i2cBmp180.read(addr_reg, data, 2);
	int utemp = (data[0] << 8) | data[1];

	//qDebug() << utemp;

	if(m_control_module){
		m_control_module->set_temperature(utemp);
	}

}

void barowork::write_baro_get_pressure()
{
	u_char addr_reg = 0xf4;

	u_char val_reg = 0x34 + (oss_bmp180 << 6);

	m_i2cBmp180.write(addr_reg, &val_reg, 1);
}

void barowork::read_baro_get_pressure()
{
	u_char addr_read = 0xf6;

	u_char data[3];

	int res = m_i2cBmp180.read(addr_read, data, sizeof(data));

	int64_t tick = get_curtime_msec();

	if(res <= 0)
		return;

	int pressure = ((data[0] << 16) + (data[1] << 8) + data[2]) >> (8 - oss_bmp180);

	if(m_control_module){
		m_control_module->set_barometer(pressure, tick);
	}
}

void barowork::set_control_module(control_module *cm)
{

}

void barowork::close()
{
	m_done = true;
}

void barowork::run()
{
	init();
	while(!m_done){
		get_data();
		_msleep(m_baro_interval);
	}
}

