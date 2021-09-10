#pragma once
#include "ILogger.h"

namespace Debug::Loggers {
	class FolderLogger : public ILogger {
		std::string m_outDir;
		size_t m_espLen;
	public:
		FolderLogger(std::string outDir, size_t espLen);
		~FolderLogger();

		bool Log(PTR ptr, DWORD exCode, PCONTEXT db) override;
	};
}