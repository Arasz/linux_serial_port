/*
 * serial_port.h
 *
 *  Created on: Dec 13, 2015
 *      Author: rafal
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <termios.h> // Library used for tty devices configuration

#include <fcntl.h> // File control library
#include <sys/types.h> // System data types
#include <sys/stat.h> // Defines the structure of the data returned by the function stat() ( file status)
#include <string>
#include <iostream>
#include <exception>
#include <unistd.h>
#include <vector>

namespace mrobot
{
	using namespace std;

	enum class parity_option
	{
		odd, // Parity bit is set to 1 if the number of ones in a given set is odd
		even, // Parity bit is set to 0 if the number of ones in a given set is even
		none, // No parity checking
	};

	enum class stop_bits_option
	{
		one,
		two,
	};

	enum class data_bits_option
	{
		six = CS6,
		seven = CS7,
		eight = CS8,
	};

	enum class baudrate_option
	{
		b0 = B0,
		b50 = B50,
		b75 = B75,
		b110 = B110,
		b134 = B134,
		b150 = B150,
		b220 = B200,
		b300 = B300,
		b600 = B600,
		b1200 = B1200,
		b1800 = B1800,
		b2400 = B2400,
		b4800 = B4800,
		b9600 = B9600,
		b19200 = B19200,
		b38400 = B38400,
		b57600 = B57600,
		b115200	= B1152000,
	};


	/**
	 * @brief  Represents exceptions during operation on files
	 */
	class serial_port_exception: public exception
	{
		//TODO: Add more parameters to file exception (fd, flags,...)
	public:
		serial_port_exception(string message, int file_descriptor=0, int flags =0):
			_message(message),_file_descriptor(file_descriptor){}

		const char* what() const throw() override {return _message.c_str();}
		const int file_descriptor(){return _file_descriptor;}
	private:
		const string _message;
		const int _file_descriptor;
	};

	/**
	 * @brief Class with allows easy serial port communication in linux.
	 */
	class serial_port
	{
	public:
		serial_port(string device, baudrate_option baudrate = baudrate_option::b9600, data_bits_option data_bits = data_bits_option::eight,
				parity_option parity = parity_option::none, stop_bits_option stop_bits=stop_bits_option::one);
		virtual ~serial_port();

		void open_device(string device); /// Open serial port device file
		void configure(baudrate_option baudrate, data_bits_option data_bits,
				parity_option parity, stop_bits_option stop_bits); /// Configure opened serial port
		void send_data(char* buffer, int length);
		int receive_data(char* buffer, int length);

	private:
		const string _device; /// Path to device
		int _file_descriptor; /// Device file descriptor
	};
} /* namespace mrobot */

#endif /* SERIALPORT_H_ */
