/*
 * iobserver.h
 *
 *  Created on: Jan 12, 2016
 *      Author: rafal
 */

#ifndef INC_IFILE_DESCRIPTOR_OWNER_H_
#define INC_IFILE_DESCRIPTOR_OWNER_H_

namespace mrobot
{
	/**
	 * @brief Interface from which will derivative all class which can be updated and have fille descriptor
	 */
	class ifile_descriptor_owner
	{
	public:
		virtual void process_data()=0; /// process data which arrived to file
		virtual int get_file_descriptor()=0; /// gets file descriptor
		virtual ~ifile_descriptor_owner() {};
	};
}



#endif /* INC_IFILE_DESCRIPTOR_OWNER_H_ */
