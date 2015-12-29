#include "control_module.h"

#include <boost/bind.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "struct_controls/struct_controls.h"

#include "utils.h"
#include <sstream>

using namespace std;
using namespace boost::asio::ip;

const int delay_timer = 5;					/// in milliseconds
const int64_t minimum_delay_for_send = 1;	/// in milliseconds

control_module::control_module()
	: m_host("0.0.0.0")
	, m_port(7777)
	, m_socket(0)
	, m_done(false)
	, m_start_send(false)
{
//	datastream stream(&m_packet);

//	m_sc.bank = 45;
//	m_sc.servo_ctrl.flag_start = true;
//	m_sc.tangaj = 30;
//	m_sc.power_on = true;
//	m_sc.servo_ctrl.pin = 3;

//	m_sc.write_to(stream);

//	sc::StructControls scc;

//	datastream ostream(m_packet);
//	scc.read_from(ostream);

//	cout << "size sc " << m_packet.size() << "\n";
}

control_module::~control_module()
{
	if(m_socket)
		delete m_socket;
}

void control_module::set_address(const std::string &address, ushort port)
{
	if(!m_socket){
		cout << "socket not created\n";
		return;
	}

	m_host = address;
	m_port = port;

	m_socket->close();
	m_socket->open(udp::v4());

	m_socket->bind(udp::endpoint(boost::asio::ip::address::from_string(address), port));
}

std::string control_module::host() const
{
	return m_host;
}

ushort control_module::port() const
{
	return m_port;
}

std::string control_module::remote_host() const
{
	return m_remote_endpoint.address().to_string();
}

ushort control_module::remote_port() const
{
	return m_remote_endpoint.port();
}

void control_module::close()
{
	m_done = true;
}

//// run thread //////////
void control_module::run()
{
	cout << "control_module start\n";

	m_socket = new udp::socket(m_io, udp::endpoint(udp::v4(), m_port));

	start_receive();

	m_last_send_data = get_curtime_msec();

	m_io.run();

	cout << "control_module stop\n";
}

void control_module::handleReceive(const boost::system::error_code &error, size_t size)
{
	size_t available = m_socket->available();
	m_buffer.resize(available);

	size_t packetSize = m_socket->receive_from(boost::asio::buffer(m_buffer, available), m_remote_endpoint);

	std::vector< char > packet;
	packet.resize(packetSize);
	std::copy(m_buffer.begin(), m_buffer.begin() + packetSize, packet.begin());

	stringstream ss;
	for(size_t i = 0; i < std::min(packetSize, (size_t)10); i++){
		ss << (uint)packet[i] << ", ";
	}

	analyze_data(packet);

	cout << "byte received: " << packetSize << "; error code: " << error;
	cout << "; packet size: " << packet.size() << " data: [" << ss.str() << "..]" << endl;
	start_receive();
}

void control_module::send_data()
{
	if(!m_start_send)
		return;

	int64_t elapsed = get_curtime_msec();
	if(elapsed - m_last_send_data < minimum_delay_for_send){
		return;
	}
	m_last_send_data = elapsed;

	std::vector< char > data;
	datastream stream(&data);
	m_data_send.write_to(stream);

	m_socket->async_send_to(boost::asio::buffer(data), m_remote_endpoint,
									 boost::bind(&control_module::write_handler, this,
												 boost::asio::placeholders::error,
												 boost::asio::placeholders::bytes_transferred));
}

void control_module::write_handler(const boost::system::error_code &error, size_t bytes_transferred)
{
	cout << "bytes send: " << bytes_transferred << endl;
}

void control_module::analyze_data(const std::vector<char> &packet)
{
	datastream stream(packet);
	char symb[5] = { 0 };
	if(packet.size() < 6){
		std::string ssymb(packet.data());
		if(ssymb == "START"){
			m_start_send = true;
			return;
		}
		if(ssymb == "STOP"){
			m_start_send = false;
			return;
		}
	}else{
		stream.readRawData(symb, 4);
		if(std::string(symb) == "CTRL"){
			m_last_sc = m_sc;
			m_sc.read_from(stream);

			sigctrl(m_sc);	/// signal
		}
	}
}

void control_module::start_receive()
{
	m_socket->async_receive_from(boost::asio::null_buffers(), m_remote_endpoint,
								 boost::bind(&control_module::handleReceive, this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
}

sc::StructControls control_module::control_params() const
{
	return m_sc;
}

void control_module::set_config_gyroscope(const sc::StructGyroscope &telem)
{
	m_config_gyroscope = telem;
}

void control_module::set_gyroscope(const vector3_::Vector3i &gyroscope,
								   const vector3_::Vector3i &accelerometer, float temp, int64_t time)
{
	sc::StructGyroscope st;
	st = m_config_gyroscope;
	st.gyro = gyroscope;
	st.accel = accelerometer;
	st.temp = temp;
	st.tick = time;

	m_mutex.lock();
	m_data_send.gyroscope = st;
	send_data();
	m_mutex.unlock();
}

void control_module::set_compass(const vector3_::Vector3i &compass, u_char mode, int64_t time)
{
	/// sensor of compass slower then gyroscope
//	qDebug() << "compass" << compass;
	m_mutex.lock();
	m_data_send.compass.data = compass;
	m_data_send.compass.mode = mode;
	m_data_send.compass.tick = time;
	send_data();
	m_mutex.unlock();
}

void control_module::set_barometer(int data, int64_t time)
{
	/// sensor of barometer very slow
	m_mutex.lock();
	m_data_send.barometer.data = data;
	m_data_send.barometer.tick = time;
	send_data();
	m_mutex.unlock();
}

void control_module::set_temperature(int temp)
{
	/// data is a part of barometer
	/// so it will be sent when the data are updated barometer

	m_mutex.lock();
	m_data_send.barometer.temp = temp;
	m_mutex.unlock();
}
