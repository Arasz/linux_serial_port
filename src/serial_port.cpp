/*
 * serial_port.cpp
 *
 *  Created on: Dec 13, 2015
 *      Author: rafal
 */

#include "serial_port.h"

namespace mrobot
{


serial_port::serial_port(string device, baudrate_option baudrate, data_bits_option data_bits,
		parity_option parity, stop_bits_option stop_bits): _device(device)
{
	// Opens device
	try
	{
		open_device(device);
		configure(baudrate, data_bits, parity, stop_bits);
	}
	catch(serial_port_exception& ex)
	{
		cerr<<ex.what();
		throw ex;
	}
}

serial_port::~serial_port()
{
	close(_file_descriptor);
}

/**
 * @brief Opens tty (serial) device given in input argument
 * By default blocking behavior is enabled.
 *
 * @param device serial device name
 * @throws serial_port_exception
 */
void serial_port::open_device(string device)
{
	/* File open flags: opens port for rw, port never becomes controlling
	 * terminal of the process, use non blocking I/O.
	 */
	int file_flags =  O_RDWR | O_NOCTTY | O_NDELAY;

	_file_descriptor = open(_device.c_str(), file_flags);
	if(_file_descriptor == -1)
	{
		throw serial_port_exception{"serial_port::open_device():\nSystem function open() can't open file.\n",_file_descriptor, file_flags};
	}
	fcntl(_file_descriptor, F_SETFL, 0); // enable blocking behavior
}
/**
 *
 * @brief Configures tty ( serial ) device.
 *
 * @param baudrate device baud rate
 * @param data_bits number of data bits
 * @param parity parity check
 * @param stop_bits number of stop bits
 *
 * @throws serial_port_exception
 */
void serial_port::configure(baudrate_option baudrate, data_bits_option data_bits, parity_option parity, stop_bits_option stop_bits)
{
	// structure used to tty device configuration
	termios config;

	// check if file descriptor is pointing to tty device
	if(!isatty(_file_descriptor))
	{
		throw serial_port_exception{"serial_port::configure():\nOpened file isn't tty device\n", _file_descriptor};
	}

	// get current configuration of the serial interface
	if(tcgetattr(_file_descriptor, &config)<0)
	{
		throw serial_port_exception{"serial_port::configure():\nCannot get serial interface configuration\n", _file_descriptor};
	}

	 // Input flags - Turn off input processing
	 // convert break to null byte, no CR to NL translation,
	 // no NL to CR translation
	 config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |INLCR);

	 // disable software flow control
	 config.c_iflag &= ~(IXON | IXOFF | IXANY);

	 // output flags - turn off output processing - raw output
	 config.c_oflag &= ~(OPOST);


	 // no line processing - raw input
	 // echo off, echo newline off, canonical mode off,
	 // extended input processing off, signal chars off
	 config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);


	 // configure parity
	 switch(parity)
	 {
	 case parity_option::none:
		 config.c_cflag &= ~PARENB; // disable parity check
		 config.c_iflag &= ~(INPCK|PARMRK|ISTRIP); // disable input parity check and parity bit strip
		 break;
	 case parity_option::odd:
		 config.c_cflag |= (PARENB|PARODD); // enable odd parity
		 config.c_iflag |= (INPCK|PARMRK|ISTRIP); // enable input parity check and parity bit strip
		 break;
	 case parity_option::even:
		 config.c_cflag |= PARENB; // enable even parity
		 config.c_cflag &= ~PARODD;
		 config.c_iflag |= (INPCK|PARMRK|ISTRIP); // enable input parity check and parity bit strip
		 break;
	 }

	 // configure stop bits
	 switch(stop_bits)
	 {
	 case stop_bits_option::one:
		 config.c_cflag &= ~CSTOPB;
		 break;
	 case stop_bits_option::two:
		 config.c_cflag |= CSTOPB;
		 break;
	 }

	 // configure data bits
	 config.c_cflag &= ~CSIZE;
	 config.c_cflag |= static_cast<unsigned int>(data_bits);
	 // TODO change configuration to consent with this at this site   http://www.cmrr.umn.edu/~strupp/serial.html#3_1

     // driver will read input bytes
	 // program won't 'own' port
 	 config.c_cflag |= (CLOCAL|CREAD);

 	 // set up control characters
	 config.c_cc[VMIN]  = 1; // one input byte is enough to return from read()
	 config.c_cc[VTIME] = 0; // inter character timer off

	 // communication speed
	 if(cfsetispeed(&config,static_cast<unsigned int>(baudrate)) < 0 || cfsetospeed(&config, static_cast<unsigned int>(baudrate)) < 0)
	 {
		 throw serial_port_exception{"serial_port::configure():\nError when setting baud rate.\n", _file_descriptor};
	 }

	 // apply the configuration ( flush buffers and apply )
	 if(tcsetattr(_file_descriptor, TCSAFLUSH, &config) < 0)
	 {
		 throw serial_port_exception{"serial_port::configure():\nCannot apply new configuration.\n", _file_descriptor};
	 }

}

/**
 * @brief Sends data through serial port
 *
 * This function counts data in input_buffer, then
 * creates temporary char array with will hold all input data.
 * After input data is copied to array it will be written to
 * serial device file by system function write(). If write
 * is successful buffer will be flushed.
 * @param input_buffer holds data to send
 * @throws serial_port_exception
 */
void serial_port::send_data(char* buffer, int length)
{
	// write data to file
	int written_bytes = write(_file_descriptor, buffer, length);

	if(written_bytes < 0)
		throw serial_port_exception{"serial_port::send_data():\nError when sending data.\n", _file_descriptor};
	else if(written_bytes < length)
		throw serial_port_exception{"serial_port::send_data():\nLess elements written than expected.\n", _file_descriptor};
}
/**
 * @brief Receives data from serial port
 *
 * This function creates buffer of size defined
 * in '_max_read_bytes_amount then reads data
 * from serial device. If there is more data to
 * read, this function is called recurrently.
 *
 * @param output_buffer holds received data
 * @throws serial_port_exception
 * @return read bytes count
 */
int serial_port::receive_data(char* buffer, int length)
{

	int read_bytes = read(_file_descriptor, buffer, length);

	if(read_bytes < 0)
		throw serial_port_exception{"serial_port::receive_data():\nError when reading data from serial port", _file_descriptor};
	return read_bytes;
}

} /* namespace mrobot */
