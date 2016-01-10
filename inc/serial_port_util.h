/*
 * serial_port_util.h
 *
 *  Created on: Jan 8, 2016
 *      Author: rafal
 */

#ifndef INC_SERIAL_PORT_UTIL_H_
#define INC_SERIAL_PORT_UTIL_H_

namespace mrobot
{
enum class parity_option
{
	odd, // Parity bit is set to 1 if the number of ones in a given set is odd
	even, // Parity bit is set to 0 if the number of ones in a given set is even
	none, // No parity checking
};

enum class stop_bits_option
{
	one, two,
};

enum class data_bits_option
{
	six = CS6, seven = CS7, eight = CS8,
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
	b115200 = B1152000,
};

}

#endif /* INC_SERIAL_PORT_UTIL_H_ */
