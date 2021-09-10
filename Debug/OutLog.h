#pragma once
#include "StackTrace.h"
#include <Zydis/Zydis.h>
#include <Poco/FileStream.h>
#include <Poco/StreamChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <cstdlib>
#include <sstream>

#define CODE_SMALL_SIZE 256
#define CODE_BLOCK_SIZE 1024

namespace Debug {
	namespace Log {
		/* Log main information for exception */
		void LogMainInfo(Poco::Logger& root, PTR ptr, DWORD exCode, PCONTEXT db, const Poco::DateTime& dt);
		/* Log call-stack for exception, return stack-end ptr */
		PTR LogCallStack(Poco::Logger& root, PTR ptr, PCONTEXT db, PTR& codePtr);
		/* Log part of code */
		void LogCode(Poco::Logger& root, PTR ptr, DWORD64 size);

		/* Log ESP Register */
		void LogStack(Poco::Logger& root, PCONTEXT db, size_t maxSize);
	}
}