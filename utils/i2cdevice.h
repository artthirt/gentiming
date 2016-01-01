#ifndef I2CDEVICE_H
#define I2CDEVICE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>

/**
 * @brief The Device class
 *  class for a implement a job on low level with i2c
 */
class Device{
public:
	Device();
	Device(const std::string& sdev);
	~Device();
	void addRef();
	/**
	 * @brief open
	 * if device not open - open
	 * else add reference
	 * @return
	 */
	bool open();
	/**
	 * @brief close
	 * decrement reference
	 * if no reference then close device
	 */
	void close();
	/**
	 * @brief dev
	 * device identificator
	 * @return
	 */
	int dev() const;
	/**
	 * @brief set_addr
	 * set address of device
	 * @param addr
	 * @return
	 */
	int set_addr(unsigned char addr);
	/**
	 * @brief set_param
	 * @param param
	 * @param value
	 * @return
	 */
	int set_param(int param, int value);
	/**
	 * @brief write
	 * write data to address in device
	 * @param addr
	 * @param data
	 * @param len
	 * @return
	 */
	int write(unsigned char addr, unsigned char* data, int len);
	/**
	 * @brief read
	 * read data from address from device
	 * @param addr
	 * @param data
	 * @param len
	 * @return
	 */
	int read(unsigned char addr, unsigned char* data, int len);
	/**
	 * @brief is_opened
	 * @return
	 */
	bool is_opened() const;
	/**
	 * @brief name
	 * @return
	 */
	std::string name() const;
private:
	int m_ref;
	std::string m_name;
	std::vector< u_char > m_write_data;
	bool m_opened;

	mutable int m_dev;

};

/**
 * @brief The I2CInstance class
 * class for global reference to devices
 */
class I2CInstance{
public:
	/**
	 * @brief open
	 * open device and return reference to device class
	 * @param sdev
	 * @return
	 */
	Device* open(const std::string& sdev);
	/**
	 * @brief close
	 * close device
	 * @param sdev
	 */
	void close(const std::string& sdev);

	static I2CInstance& instance();
private:
	static I2CInstance m_instance;

	std::map< std::string, Device > m_devices;
};

/**
 * @brief The I2CDevice class
 * class for implement a job with i2c devices
 */
class I2CDevice
{
public:
	enum{
		UNKNOWN = 0xff
	};
	I2CDevice(std::string device = "/dev/i2c-1", u_char dev = UNKNOWN, bool tenbits = false);
	~I2CDevice();

	u_char addr() const;

	std::string device() const;

	bool is_opened() const;
	/**
	 * @brief open
	 * open device
	 * @param addr
	 * @param tenbits
	 * @return
	 */
	bool open(u_char addr, bool tenbits = false);
	/**
	 * @brief close
	 * close device
	 */
	void close();

	int write(u_char addr, u_char data[], int count);
	int read(u_char addr, u_char data[], int count);

private:
	Device* m_i2cdev;
	u_char m_addr;
	std::string m_device;
	static std::mutex m_mutex;
};

#endif // I2CDEVICE_H
