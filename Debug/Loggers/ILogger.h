#pragma once
#include "../OutLog.h"

namespace Debug::Loggers {
	class ILogger {
	public:
		virtual ~ILogger() = default;
		/* Return true if need continue execution */
		virtual bool Log(PTR ptr, DWORD exCode, PCONTEXT db) = 0;
	};
}