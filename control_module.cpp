#include "control_module.h"

#include <boost/bind.hpp>

#include "struct_controls/struct_controls.h"

#include "utils.h"
#include <sstream>

using namespace std;
using namespace boost::asio::ip;

control_module::control_module()
	: m_host("0.0.0.0")
	, m_port(7777)
	, m_socket(0)
	, m_done(false)
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

void control_module::set_address(const std::__cxx11::string &address, ushort port)
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

void control_module::run()
{
	cout << "control_module start\n";

	m_socket = new udp::socket(m_io, udp::endpoint(udp::v4(), m_port));

	start_receive();

	m_io.run();

	cout << "control_module stop\n";
}

std::vector<char> control_module::packet() const
{
	return m_packet;
}

void control_module::handleReceive(const boost::system::error_code &error, size_t size)
{
	size_t available = m_socket->available();
	m_buffer.resize(available);

	size_t packetSize = m_socket->receive_from(boost::asio::buffer(m_buffer, available), m_remote_endpoint);

	m_packet.resize(packetSize);
	std::copy(m_buffer.begin(), m_buffer.begin() + packetSize, m_packet.begin());

	stringstream ss;
	for(size_t i = 0; i < std::min(packetSize, (size_t)10); i++){
		ss << (uint)m_packet[i] << ", ";
	}

	cout << "byte received: " << packetSize << "; error code: " << error;
	cout << "; packet size: " << m_packet.size() << " data: [" << ss.str() << "..]" << endl;
	start_receive();
}

void control_module::start_receive()
{
	m_socket->async_receive_from(boost::asio::null_buffers(), m_remote_endpoint,
								 boost::bind(&control_module::handleReceive, this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
}


