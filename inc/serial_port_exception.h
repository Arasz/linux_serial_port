/*
 * serial_port_exception.h
 *
 *  Created on: Jan 8, 2016
 *      Author: rafal
 */

#ifndef INC_SERIAL_PORT_EXCEPTION_H_
#define INC_SERIAL_PORT_EXCEPTION_H_
#include <string>
#include <exception>

namespace mrobot
{

/**
 * @brief  Represents exceptions during operation on files
 */
class serial_port_exception: public std::exception
{

	//TODO: Add more parameters to file exception (fd, flags,...)
public:
	serial_port_exception(std::string message, std::string error_description = "None") :
			_message(message), _error_description(error_description)
	{
	}

	const char* what() const throw () override
	{
		std::string message= "Message: "+_message+"\nError: "+_error_description+"\n";
		return message.c_str();
	}
private:
	const std::string _message;
	const std::string _error_description;
};
}
#endif /* INC_SERIAL_PORT_EXCEPTION_H_ */
