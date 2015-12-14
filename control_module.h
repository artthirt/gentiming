#ifndef CONTROL_MODULE_H
#define CONTROL_MODULE_H

#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include "struct_controls/struct_controls.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <boost/asio/basic_socket_streambuf.hpp>

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

	sc::StructGyroscope config_gyroscope() const;
	sc::StructControls control_params() const;
	void set_config_gyroscope(const sc::StructGyroscope& telem);
	void set_port_receiver(ushort port);
	void set_gyroscope(const vector3_::Vector3i& gyroscope,
					   const vector3_::Vector3i& accelerometer, float temp, int64_t time);
	void set_compass(const vector3_::Vector3i& compass, u_char mode, int64_t time);
	void set_barometer(int data, int64_t time);
	void set_temperature(int temp);


protected:
	void handleReceive(const boost::system::error_code& error,
					   std::size_t size);
	void start_receive();
	void send_data();
	void write_handler(const boost::system::error_code& error, // Result of operation.
					   std::size_t bytes_transferred);

private:
	std::string m_host;
	ushort m_port;
	std::vector< char > m_buffer;
	std::vector< char > m_packet;
	bool m_done;
	sc::StructControls m_sc;
	sc::StructTelemetry m_data_send;
	sc::StructGyroscope m_config_gyroscope;
	std::mutex m_mutex;
	int64_t m_last_send_data;

	boost::asio::ip::udp::socket *m_socket;
	boost::asio::ip::udp::endpoint m_remote_endpoint;
	boost::asio::io_service m_io;
};

#endif // CONTROL_MODULE_H
