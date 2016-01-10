/*
 * main.cpp
 *
 *  Created on: Dec 13, 2015
 *      Author: rafal
 */

#include "serial_port.h"
#include <string>

void default_config_test()
{
	using namespace std;

	string name = "/dev/ttyS98";
	try
	{
		mrobot::serial_port serial_device{name};
	}
	catch(mrobot::serial_port_exception& ex)
	{
		cout<<"default_config_test() failed - exception was thrown: "<<ex.what()<<endl;
		return;
	}
	cout<<"default_config_test() succeed"<<endl;
}

void config_test()
{
	using namespace std;
	using namespace mrobot;

	string name = "/dev/ttyS98";
	baudrate_option baud = baudrate_option::b115200;
	data_bits_option data = data_bits_option::eight;
	parity_option parity = parity_option::none;
	stop_bits_option stop_bits = stop_bits_option::one;

	try
	{
		serial_port serial_device(name, baud, data, parity, stop_bits);
	}
	catch(serial_port_exception& ex)
	{
		cout<<"config_test() failed - exception was thrown: "<<ex.what()<<endl;
		return;
	}
	cout<<"config_test() succeed"<<endl;
}

void echo(mrobot::serial_port& serial_device, std::vector<char>& data)
{
	using namespace mrobot;
	if(serial_device.is_ready())
	{
		try
		{
			std::cout<<"Sent data: ";
			for(char&c:data)
			{
				std::cout<<c;
			}
			std::cout<<"\n";

			serial_device.send_data(data);
		}
		catch(serial_port_exception& ex)
		{
			std::cerr<<ex.what()<<std::endl;
		}
	}
}



void action_test()
{
	using namespace std;
	using namespace mrobot;

	string name = "/dev/ttyS98";
	baudrate_option baud = baudrate_option::b115200;
	data_bits_option data = data_bits_option::eight;
	parity_option parity = parity_option::none;
	stop_bits_option stop_bits = stop_bits_option::one;

	try
	{
		serial_port serial_device(name, baud, data, parity, stop_bits);

		serial_port::data_ready_event_handler read_data_handler = echo;
		serial_device.subscribe_data_ready_event(read_data_handler);


		vector<char> buffer {'S','T','A','R','T',' ','E','C','H','O',' ','T','E','S','T',':',' ','\n'};

		cout<<serial_device.is_open()<<endl;
		cout<<serial_device.is_configured()<<endl;
		cout<<serial_device.is_ready()<<endl;

		if(serial_device.is_ready())
			serial_device.send_data(buffer);
		while(true)
		{

		}

	}
	catch(serial_port_exception& ex)
	{
		cout<<"action_test() failed - exception was thrown: "<<ex.what()<<endl;
		return;
	}
	cout<<"action_test() succeed"<<endl;
}

void action_test_2()
{
	using namespace std;
	using namespace mrobot;

	string name = "/dev/ttyS98";
	baudrate_option baud = baudrate_option::b115200;
	data_bits_option data = data_bits_option::eight;
	parity_option parity = parity_option::none;
	stop_bits_option stop_bits = stop_bits_option::one;

	try
	{
		serial_port serial_device(name, baud, data, parity, stop_bits);

		//serial_port::data_ready_event_handler read_data_handler = echo;
		//serial_device.subscribe_data_ready_event(read_data_handler);


		vector<char> buffer {'S','T','A','R','T',' ','E','C','H','O',' ','T','E','S','T',':',' ','\n'};

		if(serial_device.is_ready())
			serial_device.send_data(buffer);
		while(serial_device.is_ready())
		{
			if(serial_device.is_data_ready()>0)
			{
				serial_device.receive_data(buffer);
				cout<<"Received data: ";
				for(char&c:buffer)
					cout<<c;
				cout<<endl;
				serial_device.send_data(buffer);
			}
		}

	}
	catch(serial_port_exception& ex)
	{
		cout<<"action_test() failed - exception was thrown: "<<ex.what()<<endl;
		return;
	}
	cout<<"action_test() succeed"<<endl;
}

int main()
{
	default_config_test();
	config_test();
	action_test();
	return 0;
}


