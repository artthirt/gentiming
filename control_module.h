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

	std::string remote_host() const;
	ushort remote_port() const;

	void close();

	void run();

	std::vector< char > packet() const;

protected:
	void handleReceive(const boost::system::error_code& error,
					   std::size_t size);
	void start_receive();

private:
	std::string m_host;
	ushort m_port;
	std::string m_remote_host;
	ushort m_remote_port;
	std::vector< char > m_buffer;
	std::vector< char > m_packet;
	bool m_done;

	boost::asio::ip::udp::socket *m_socket;
	boost::asio::ip::udp::endpoint m_remote_endpoint;
	boost::asio::io_service m_io;
};

#endif // CONTROL_MODULE_H
