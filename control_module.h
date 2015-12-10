#ifndef CONTROL_MODULE_H
#define CONTROL_MODULE_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/array.hpp>

class control_module
{
public:
	control_module();
	~control_module();

	void set_address(const std::string& address, ushort port);
	std::string host() const;
	ushort port() const;

	std::string sender_host() const;
	ushort sender_port() const;

	void close();

	void run();

protected:
	void handleReceive(const boost::system::error_code& error,
					   std::size_t size);
	void start_receive();

private:
	std::string m_host;
	ushort m_port;
	std::string m_sender_host;
	ushort m_sender_port;
	boost::array< char, 2048 > m_buffer;
	bool m_done;

	boost::asio::ip::udp::socket *m_socket;
	boost::asio::ip::udp::endpoint m_remote_endpoint;
	boost::asio::io_service m_io;
};

#endif // CONTROL_MODULE_H
