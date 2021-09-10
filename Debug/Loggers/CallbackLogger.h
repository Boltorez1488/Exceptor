#pragma once
#include "ILogger.h"

namespace Debug::Loggers {
	class CallbackLogger : public ILogger {
		bool(*m_callback)(const std::string&);
		size_t m_espLen;
	public:
		CallbackLogger(bool(*callback)(const std::string&), size_t espLen);
		~CallbackLogger();

		bool Log(PTR ptr, DWORD exCode, PCONTEXT db) override;
	};
}