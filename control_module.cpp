#include "control_module.h"

#include <boost/bind.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "struct_controls/struct_controls.h"

#include "utils.h"
#include <sstream>

using namespace std;
using namespace boost::asio::ip;

const int delay_timer = 10;					/// in milliseconds
const int64_t minimum_delay_for_send = 1;	/// in milliseconds

//////////////////////////////////////////////////

void timer_handler(sigval_t sv)
{
	//long tm = get_curtime_usec();
	control_module* cm = static_cast< control_module* >(sv.sival_ptr);
	if(cm){
		cm->handler_timer();
	}
//	static long time_last = 0;
//	std::cout << tm - time_last << endl;
//	time_last = tm;
}

//////////////////////////////////////////////////

control_module::control_module()
	: m_host("0.0.0.0")
	, m_port(7777)
	, m_socket(0)
	, m_done(false)
	, m_start_send(false)
	, m_delay(delay_timer)
	, m_is_data_update(false)
{
//	std::vector< char > packet;
//	datastream stream(&packet);

//	m_sc.bank = 45;
//	m_sc.servo_ctrl.flag_start = true;
//	m_sc.tangaj = 30;
//	m_sc.power_on = true;
//	m_sc.servo_ctrl.pin = 3;
//	sc::StructTelemetry s1;
//	s1.compass.data = vector3_::Vector3i(1, 2, 3);

//	s1.write_to(stream);

//	sc::StructTelemetry scc;
//	sc::StructControls scc;

//	datastream ostream(packet);
//	scc.read_from(ostream);

//	cout << "size sc " << packet.size() << "\n";
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

	start_timer();

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

	m_is_data_update = true;
}

void control_module::write_handler(const boost::system::error_code &error, size_t bytes_transferred)
{
	if(error != 0){
		cout << m_remote_endpoint.address().to_string() << " " << m_remote_endpoint.port() << endl;
		cout << "bytes send: " << bytes_transferred << "; error: " << error << endl;
	}
}

void control_module::analyze_data(const std::vector<char> &packet)
{
	datastream stream(packet);
	char symb[6] = { 0 };
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

void control_module::start_timer()
{
	int res = 0;
	std::fill((char*)&m_event, (char*)&m_event + sizeof(m_event), '\0');
	m_event.sigev_value.sival_ptr = this;
	m_event.sigev_notify = SIGEV_THREAD;
	m_event.sigev_notify_function = timer_handler;
	res = timer_create(CLOCK_MONOTONIC, &m_event, &m_timer_id);
	cout << "timer_create: " << res << endl;

	struct itimerspec tm = { get_from_ms(m_delay), get_from_ms(m_delay) };
	//tm.it_interval.tv_nsec = (long)(m_delay * (1e+3)) % (long)1e+9;
	//tm.it_interval.tv_sec = m_delay / 1000;
	res = timer_settime(m_timer_id, 0, &tm, 0);
	cout << "timer_settime: " << res << endl;
}

void control_module::handler_timer()
{
	if(m_is_data_update){
		if(!m_remote_endpoint.port()){
			m_is_data_update = false;
			return;
		}

		std::vector< char > data;
		datastream stream(&data);
		m_data_send.write_to(stream);

		m_socket->async_send_to(boost::asio::buffer(data), m_remote_endpoint,
										 boost::bind(&control_module::write_handler, this,
													 boost::asio::placeholders::error,
													 boost::asio::placeholders::bytes_transferred));

		m_is_data_update = false;
	}
}

void control_module::start_receive()
{
	boost::asio::ip::udp::endpoint remote_endpoint;
	m_socket->async_receive_from(boost::asio::null_buffers(), remote_endpoint,
								 boost::bind(&control_module::handleReceive, this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
	//cout << remote_endpoint.address().to_string() << " " << remote_endpoint.port() << endl;
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
