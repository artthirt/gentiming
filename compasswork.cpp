#include "compasswork.h"
#include <vector>

#include "utils.h"
#include "vector3_.h"
#include "struct_controls.h"
#include "control_module.h"

using namespace sc;
using namespace vector3_;
using namespace std;

const u_char confRegA_hmc5883 = 0x00;
const u_char confRegB_hmc5883 = 0x01;
const u_char confRegM_hmc5883 = 0x02;

compasswork::compasswork()
	: m_done(false)
	, m_control_module(0)
	, m_regA_hmc5883(0)
{

}

void compasswork::init()
{
	if(!m_i2cHmc5883.open(addr_hmp5883)){
		std::cout << "hmp5883l: device not open\n";
		return;
	}
	std::cout << "hmc5883: device opened\n";

	const u_char ma10 = 0x3;		/// 4 samples averages per measurement output;
	const u_char do210 = 0x6;	/// mode of output rate (0x6 === 75Hz)
	const u_char ms10 = 0;		/// measurement mode (0 === normal)

	m_regA_hmc5883 =  (ma10 << 5) | (do210 << 2) | (ms10);

	m_i2cHmc5883.write(confRegA_hmc5883, &m_regA_hmc5883, 1);

	u_char config_regB = 0x1 << 5;//	/// 1.3 Ga (default)

	m_i2cHmc5883.write(confRegB_hmc5883, &config_regB, 1);

	u_char mode = 0x0;			/// continuous mode

	m_i2cHmc5883.write(confRegM_hmc5883, &mode, 1);
}

void compasswork::get_data()
{
	if(!m_i2cHmc5883.is_opened())
		return;
	int res = 0;
	u_char data_out[6] = { 0 };
	short data[3] = { 0 };
	u_char addr_data_hmc5883 = 0x03;

	res = m_i2cHmc5883.read(addr_data_hmc5883, data_out, sizeof(data_out));

//	qDebug() << "compass raw" << (int)m_i2cHmc5883.addr() << data_out[0] << data_out[1] << data_out[2] << res;

	int64_t tick = get_curtime_msec();

	if(res <= 0)
		return;

	for(int i = 0; i < 3; i++){
		data[i] = (data_out[(i << 1)] << 8) | (data_out[(i << 1) + 1]);
	}

	Vector3i out(data[0], data[2], data[1]);
	//out += Vector3i(-80, 150, -410);

//	double d = out.x() * out.x() + out.y() * out.y();

//	qDebug() << out << sqrt(d);

	if(m_control_module){
		m_control_module->set_compass(out, m_regA_hmc5883, tick);
	}

}

void compasswork::set_control_module(control_module *cm)
{
	m_control_module = cm;
}

void compasswork::close()
{
	m_done = true;
}

void compasswork::run()
{
	init();
	while(!m_done){
		get_data();
		_msleep(period_gyroAndCompass);
	}
}

