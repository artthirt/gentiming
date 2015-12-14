#include "gyrowork.h"

#include "struct_controls.h"
#include "vector3_.h"
#include "utils.h"

#include "control_module.h"

using namespace sc;
using namespace vector3_;

const u_char gyro_address_data = 0x3b;
const u_char raw_data_address = 0x0d;

gyrowork::gyrowork()
	: m_control_module(0)
	, m_done(false)
{

}

void gyrowork::init()
{
	if(!m_i2cMpu6050.open(addr_mpu6050)){
		std::cout << "mpu6050: device not open\n";
		return;
	}
	std::cout << "mpu6050: device opened\n";
	u_char data[2] = { 0 };
	m_i2cMpu6050.write(0x6B, data, 1);

	if(m_control_module){
		StructGyroscope gyroscope;

		m_i2cMpu6050.read(raw_data_address, gyroscope.raw, raw_count);
		/// get flags from raw data (used datasheet for registers)
		gyroscope.afs_sel =	(gyroscope.raw[28 - raw_data_address] >> 3) & 0x03;
		gyroscope.fs_sel =	(gyroscope.raw[27 - raw_data_address] >> 3) & 0x03;

		u_char dlpf_cfg =	gyroscope.raw[26 - raw_data_address] & 0x3;
		int smplrt_div =	gyroscope.raw[25 - raw_data_address];
		int gyr_out_rate;

		switch (dlpf_cfg) {
			case 0:
			case 7:
				gyr_out_rate = 8000;
				break;
			default:
				gyr_out_rate = 1000;
				break;
		}

		double sample_rate_div = gyr_out_rate / (1 + smplrt_div);

		gyroscope.freq =  sample_rate_div;

		m_control_module->set_config_gyroscope(gyroscope);
	}
}

void gyrowork::get_data()
{
	if(!m_i2cMpu6050.is_opened())
		return;

	int res;

	u_short data_out[7];
	short data[7];

	res = m_i2cMpu6050.read(gyro_address_data, reinterpret_cast<u_char *>(data_out), 14);

	int64_t tick = get_curtime_msec();

	for(int i = 0; i < 7/*data_out.size()*/; i++){
		data[i] = static_cast<short>((data_out[i] << 8) | (data_out[i] >> 8));
	}

	if(m_control_module){
		m_control_module->set_gyroscope(Vector3i(data[4], data[5], data[6]),
				Vector3i(data[0], data[1], data[2]),
				data[3] / 340.f + 36.53f, tick);
	}
}

void gyrowork::set_control_module(control_module *cm)
{
	m_control_module = cm;
}

void gyrowork::close()
{
	m_done = true;
}

void gyrowork::run()
{
	init();
	while(!m_done){
		get_data();
		_msleep(period_gyroAndCompass);
	}
}
