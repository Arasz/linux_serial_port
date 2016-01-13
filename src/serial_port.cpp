/*
 * serial_port.cpp
 *
 *  Created on: Dec 13, 2015
 *      Author: rafal
 */

#include "serial_port.h"

namespace mrobot
{


serial_port::serial_port(std::string device, baudrate_option baudrate, data_bits_option data_bits,
		parity_option parity, stop_bits_option stop_bits): _device(device)
{
	open_device(device);
	configure(baudrate, data_bits, parity, stop_bits);
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
void serial_port::open_device(std::string device)
{
	/* File open flags: opens port for rw, port never becomes controlling
	 * terminal of the process, use non blocking I/O.
	 */
	int file_flags =  O_RDWR | O_NOCTTY | O_NDELAY;

	_file_descriptor = open(_device.c_str(), file_flags);
	if(_file_descriptor == -1)
	{
		throw serial_port_exception{"System function open() can't open file.", strerror(errno)};
	}
	fcntl(_file_descriptor, F_SETFL, 0); // enable blocking behavior
	_is_opend = true;
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
		throw serial_port_exception("Opened file isn't tty device", strerror(errno));
	}

	// get current configuration of the serial interface
	if(tcgetattr(_file_descriptor, &config)<0)
	{
		throw serial_port_exception("Cannot get serial interface configuration", strerror(errno));
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
		 throw serial_port_exception("Error when setting baud rate.", strerror(errno));
	 }

	 // apply the configuration ( flush buffers and apply )
	 if(tcsetattr(_file_descriptor, TCSAFLUSH, &config) < 0)
	 {
		 throw serial_port_exception("Cannot apply new configuration.", strerror(errno));
	 }
	 _is_configured = true;
}


/**
 * @brief Sends data through serial port
 *
 * This function counts data in buffer, then
 * creates temporary char array with will hold all input data.
 * After input data is copied to array it will be written to
 * serial device file by system function write(). If write
 * is successful buffer will be flushed.
 * @param buffer holds data to send
 * @throws serial_port_exception
 */
void serial_port::send_data(const std::vector<char>& buffer)
{
	//std::unique_lock<std::mutex> lock{_fd_mutex};

	int length = buffer.size();
	char out_buffer[length];

	int i = 0;
	for (const char&c : buffer)
	{
		out_buffer[i++] = c;
	}

	// write data to file
	int written_bytes = write(_file_descriptor, out_buffer, length);

	if (written_bytes < 0)
		throw serial_port_exception("Error when sending data.",
				strerror(errno));
	else if (written_bytes < length)
		throw serial_port_exception("Less elements written than expected.",
				strerror(errno));
}


/**
 * @brief Subscribe data ready event
 * @param event_handler function which will handle data ready event
 */
void serial_port::subscribe_data_ready_event(data_ready_event_handler& event_handler)
{
	if(!_is_data_ready_event_subscribed)
	{
		_data_ready_event_handler = event_handler;
		_is_data_ready_event_subscribed = true;
	}
}

void serial_port::unsubscribe_data_ready_event()
{
	if(_is_data_ready_event_subscribed)
	{
		_is_data_ready_event_subscribed = false;
	}
}

/**
 * @brief Read data from serial port
 */
void serial_port::read_data()
{
	//std::unique_lock<std::mutex> lock{_fd_mutex};

	char read_buffer[_data_buffer_size];
	std::memset(read_buffer, 0, _data_buffer_size);

	int read_bytes = read(_file_descriptor, read_buffer, _data_buffer_size);

	if(read_bytes < 0)
		throw serial_port_exception{"Error when reading data from serial port", strerror(errno)};

	_received_data_buffer.clear();
	for(int i = 0; i<read_bytes; i++)
	{
		_received_data_buffer.push_back(read_buffer[i]);
	}
}

/**
 * @brief Checks if there is data ready to read.
 * @return size of data to read in bytes
 */
int serial_port::is_data_ready()
{
	// data ready to read
	int bytes = 0;

	ioctl(_file_descriptor, FIONREAD, &bytes); // check number of bytes ready to read

	return bytes;
}

/**
 * @brief Reads data from serial port
 * @param buffer received data buffer
 */
void serial_port::receive_data(std::vector<char>& buffer)
{
	//std::unique_lock<std::mutex> lock{_fd_mutex};

	int buffer_size = buffer.size();
	char read_buffer[buffer_size];
	std::memset(read_buffer, 0, buffer_size);

	int read_bytes = read(_file_descriptor, read_buffer, buffer_size);

	if(read_bytes < 0)
		throw serial_port_exception{"Error when reading data from serial port", strerror(errno)};

	buffer.clear();
	for(int i = 0; i<read_bytes; i++)
	{
		buffer.push_back(read_buffer[i]);
	}
}

void serial_port::process_data()
{
	read_data();
	if(_is_data_ready_event_subscribed)
		_data_ready_event_handler(*this,_received_data_buffer);
}

int serial_port::get_file_descriptor()
{
	return _file_descriptor;
}


} /* namespace mrobot */

