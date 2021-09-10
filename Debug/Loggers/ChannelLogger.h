#pragma once
#include "ILogger.h"

namespace Debug {
	namespace Loggers {
		class ChannelLogger : public ILogger {
			Poco::Channel* m_pChannel;
			size_t m_espLen;
		public:
			ChannelLogger(Poco::Channel* pChannel, size_t espLen);
			~ChannelLogger();

			bool Log(PTR ptr, DWORD exCode, PCONTEXT db) override;
		};
	}
}