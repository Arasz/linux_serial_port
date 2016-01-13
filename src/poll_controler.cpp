/*
 * poll_controler.cpp/
 *
 *  Created on: Jan 12, 2016
 *      Author: rafal
 */

#include <poll_controler.h>

namespace mrobot
{

poll_controler::poll_controler() :
		poll_controler(-1, milliseconds(500))
{

}

poll_controler::poll_controler(int poll_timeout,
		milliseconds poll_interval) :
		_timeout(poll_timeout), _poll_interval(poll_interval)
{

}

poll_controler::~poll_controler()
{
	delete[] _ufds;
	_is_poll_thread_running = false;
	_poll_thread.join();
}

void poll_controler::add(ifile_descriptor_owner* observer)
{
	_observers[observer->get_file_descriptor()] = observer;
}

void poll_controler::remove(ifile_descriptor_owner* observer)
{
	_observers.erase(observer->get_file_descriptor());
}

void poll_controler::start_polling()
{
	construct_ufds_array();
	_is_poll_thread_running = true;
	_poll_thread = std::thread
	{ &poll_controler::poll_loop, this };
}
void poll_controler::stop_polling()
{
	construct_ufds_array();
	_is_poll_thread_running = false;
	_poll_thread.join();
}

void poll_controler::poll_loop()
{
	std::cerr << "poll_loop\n";
	while (_is_poll_thread_running)
	{
		std::cerr << "insight loop...\n";
		std::this_thread::sleep_for(_poll_interval);
		try
		{
			poll_file_descriptors();
		} catch (poll_exception& ex)
		{
			std::cerr << ex.what();
		}
	}
}

void mrobot::poll_controler::poll_file_descriptors()
{
	std::cerr << "poll_file_descriptors\n";
	if (_observed_fd_count > 0)
	{
		// events_count equal to zero means timeout
		int events_count = poll(_ufds, _observed_fd_count, _timeout);
		std::cerr << "after poll\n";

		if (events_count < 0)
			throw poll_exception
			{ "Error when polling file descriptors.", strerror(errno) };
		else
		{
			for (int i = 0; (i < _observed_fd_count) && (events_count > 0);
					i++)
			{
				if (_ufds[i].events & POLLIN)
				{
					std::cerr << "data ready to read\n";
					_observers[_ufds[i].fd]->process_data();
					events_count--;
				}
			}
		}
	}
}

void mrobot::poll_controler::construct_ufds_array()
{
	//TODO trivial version of construction. Can be done better.

	_observed_fd_count = _observers.size();

	std::cerr << "Observers.size(): " << _observed_fd_count << "\n";

	if (_ufds != nullptr)
		delete[] _ufds;

	_ufds = new pollfd[_observed_fd_count];

	int i = 0;
	for (auto observer : _observers)
	{
		_ufds[i].fd = observer.first;
		std::cerr << "fd: " << observer.first << "\n";
		_ufds[i++].events = POLLIN;
	}

	_are_poll_objects_initialized = true;
}

}
