/**
 * \ingroup drivers
 * @file SHT1x.h
 *
 * SHT1x temperature and humidity sensor driver
 *
 * \date 27. 5. 2014
 * \author Vladimír Matěna <vlada@mattty.cz>
 */

#ifndef SHT1X_H
#define SHT1X_H

#include "cstdio"
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>

/**
 * SHT1x temperature and humidity sensor driver
 *
 * \ingroup drivers
 */
class SHT1x {
public:
	/// SHT1x driver settings
	struct Properties {
		/// GPIO port to use for hardware control
		GPIO_TypeDef* gpio;
		/// GPIO clock to use for hardware control
		uint32_t gpioClk;
		/// GPIO pin connected to SHT1x SCL pin
		uint32_t sclPin;
		/// GPIO pin connected to SHT1x SDA pin
		uint32_t sdaPin;
	};

	/**
	 * SHT1x driver constructor
	 */
	SHT1x(Properties &properties);

	/**
	 * SHT1x driver initialization
	 */
	void init();

	/**
	 * Read temperature from sensor
	 *
	 * This method blocks.
	 *
	 * @return Temperature in degrees celsius * 10 as signed integer
	 */
	int16_t readTemperature();

	/**
	 * Read humidity from sensor
	 *
	 * This method blocks.
	 *
	 * @return relative humidity in percents * 10 as unsigned integer
	 */
	int16_t readHumidity();

private:
	/// Temperature measure command code
	static const uint8_t CMD_MEASURE_TEMP = 0b00000011;
	/// Humidity measure command code
	static const uint8_t CMD_MEASURE_HUMID = 0b00000101;
	/// Read status register command code
	static const uint8_t CMD_READ_STATUS = 0b00000111;
	/// Write status register command code
	static const uint8_t CMD_WRITE_STATUS = 0b00000110;
	/// Soft reset command code
	static const uint8_t CMD_SOFT_RESET = 0b00011110;

	/// Communication delay in microseconds
	static const uint16_t COM_DELAY = 3;

	/// Temperature offset constant
	static constexpr double TC1 = -39.7f;
	/// Temperature multiplier constant
	static constexpr double TC2 = 0.01f;

	/// Humidity offset constant
	static constexpr double HC1 = -2.0468f;
	/// Humidity multiplier constant
	static constexpr double HC2 = 0.0367f;
	/// Humidity exponent constant
	static constexpr double HC3 = -1.5955E-6f;

	/// Driver IO properties
	Properties &properties;

	/// Reset communication
	void reset();
	/**
	 * Set SDA pin mode
	 *
	 * @param mode Mode
	 */
	void sdaMode(uint32_t mode);
	/// Set SDA pin hi
	void sdaHi();
	/// Set SDA pin lo
	void sdaLo();
	/// Read SDA pin state
	bool sdaRead();
	/// Pulse SCL pin
	void sclPulse();
	/// Set SCL pin hi
	void sclHi();
	/// Set SCL pin lo
	void sclLo();
	/**
	 * Send byte to SHT1x
	 *
	 * @param data Data to send
	 */
	bool sendByte(uint8_t data);

	/**
	 * Receive byte
	 *
	 * @return Byte received
	 */
	uint8_t receiveByte();

	/// Send start command
	void startCommand();

	/**
	 * Send command and read result
	 *
	 * @param command Command code to send
	 * @return Two bytes of command result
	 */
	uint16_t readCommand(uint8_t command);
};

#endif // SHT1X_H
