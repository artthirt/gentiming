#include "control_module.h"

#include <boost/bind.hpp>

#include "utils.h"
#include <sstream>

using namespace std;
using namespace boost::asio::ip;

control_module::control_module()
	: m_host("0.0.0.0")
	, m_port(7777)
	, m_sender_port(0)
	, m_socket(0)
	, m_done(false)
{
}

control_module::~control_module()
{
	if(m_socket)
		delete m_socket;
}

void control_module::set_address(const std::__cxx11::string &address, ushort port)
{
	m_host = address;
	m_port = port;
}

std::string control_module::host() const
{
	return m_host;
}

ushort control_module::port() const
{
	return m_port;
}

std::string control_module::sender_host() const
{
	return m_sender_host;
}

ushort control_module::sender_port() const
{
	return m_sender_port;
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
}

void control_module::handleReceive(const boost::system::error_code &error, size_t size)
{
	stringstream ss;
	for(size_t i = 0; i < std::min(size, (size_t)10); i++){
		ss << (uint)m_buffer[i] << ", ";
	}

	cout << "byte received: " << size << "; error code: " << error << endl;
	cout << "buffer size: " << m_buffer.size() << " data: [" << ss.str() << "..]" << endl;
	start_receive();
}

void control_module::start_receive()
{
	m_socket->async_receive_from(boost::asio::buffer(m_buffer), m_remote_endpoint,
								 boost::bind(&control_module::handleReceive, this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
}


