#ifndef GPIORW_H
#define GPIORW_H

#include <string>
#include <vector>
#include <map>

namespace gpio{

const std::string gpio_export("/sys/class/gpio/export");
const std::string gpio_unexport("/sys/class/gpio/unexport");

class gpiorw
{
public:
	enum DIRECTION{
		D_IN = 0,
		D_OUT = 1
	};

	gpiorw();
	~gpiorw();
	/**
	 * @brief open_pin
	 * @param pin
	 * @return
	 */
	bool open_pin(int pin);
	/**
	 * @brief close_pin
	 * @param pin
	 * @return
	 */
	bool close_pin(int pin);
	/**
	 * @brief write_to_file
	 * write to custom file
	 * @param sfile
	 * @param value
	 * @return
	 */
	bool write_to_file(const std::string& sfile, const std::string& value);
	/**
	 * @brief write_to_gpio_custom
	 * write custom string to gpio pin in key file
	 * @param pin
	 * @param key
	 * @param value
	 * @return
	 */
	bool write_to_gpio_custom(int pin, const std::string& key, const std::string& value);
	/**
	 * @brief write_to_gpio_custom
	 * write custom int to gpio pin in key file
	 * @param pin
	 * @param key
	 * @param value
	 * @return
	 */
	bool write_to_gpio_custom(int pin, const std::string& key, const int value);
	/**
	 * @brief write_to_pin
	 * write 1 or 0 to pin
	 * @param pin
	 * @param value
	 * @return
	 */
	bool write_to_pin(int pin, bool value);
	/**
	 * @brief read_from_pin
	 * read value from pin
	 * @param pin
	 * @return
	 */
	int read_from_pin(int pin);
	/**
	 * @brief set_direction_gpio
	 * @param pin
	 * @param dir
	 * @return
	 */
	bool set_direction_gpio(int pin, DIRECTION dir);
	/**
	 * @brief instance
	 * global instance
	 * @return
	 */
	static gpiorw &instance();

private:
	std::map< int, std::string > m_open_pins;

	static gpiorw m_instance;

	bool contains(int key);
	std::string path_pin(int pin) const;
	std::string pin_key_file(int pin, const std::string& key);
};

}

#endif // GPIORW_H
