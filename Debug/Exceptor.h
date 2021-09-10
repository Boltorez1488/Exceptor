#pragma once
#include <Poco/ConsoleChannel.h>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "WS2_32.lib")

namespace Debug {
	/* Powerful exception filter for crashes */
	class Exceptor {
	public:
		/* Attach exception filter to current application */
		static void Attach(Poco::Channel* pChannel, size_t espLen = 1024);
		/* Attach exception filter and report of crash saves to directory */
		static void AttachFolder(const std::string& dir = "crashes", size_t espLen = 1024);
		/* Attach exception filter and report to callback function */
		static void AttachCallbackZip(bool(*callback)(const std::string&), size_t espLen = 1024);
	};
}