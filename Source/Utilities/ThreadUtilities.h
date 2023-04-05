#ifndef _THREAD_UTILITIES_H_
#define _THREAD_UTILITIES_H_

#include <string>
#include <thread>

namespace Utilities {

	std::string getThreadName(std::thread & thread);
	void setThreadName(std::thread & thread, const std::string & threadName);

}

#endif // _THREAD_UTILITIES_H_
