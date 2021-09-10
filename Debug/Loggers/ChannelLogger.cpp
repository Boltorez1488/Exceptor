#include "ChannelLogger.h"

using namespace Debug::Loggers;

ChannelLogger::ChannelLogger(Poco::Channel* pChannel, size_t espLen) : m_pChannel(pChannel), m_espLen(espLen) {
}

ChannelLogger::~ChannelLogger() {
	m_pChannel->close();
}

bool ChannelLogger::Log(PTR ptr, DWORD exCode, PCONTEXT db) {
	Poco::Logger& root = Poco::Logger::root();
	root.setChannel(m_pChannel);

	const auto dt = Poco::DateTime();
	Log::LogMainInfo(root, ptr, exCode, db, dt);
	PTR codePtr = NULL;
	const auto stackEnd = Log::LogCallStack(root, ptr, db, codePtr);
	const auto stackLen = stackEnd - IS64(db->Rsp, db->Esp);
	if (m_espLen > stackLen)
		m_espLen = stackLen;
	if (!IsBadCodePtr((FARPROC)codePtr)) {
		root.fatal("|-----------------------------------------------------------------------");
		root.fatal(IS64("| Ptr-Asm: %016LX", "| Ptr-Asm: %08lX"), codePtr);
		root.fatal("|-----------------------------------------------------------------------");
		Log::LogCode(root, codePtr, CODE_SMALL_SIZE);
	} else {
		root.fatal("|-----------------------------------------------------------------------");
		root.fatal("| Bad executable pointer");
		root.fatal("|-----------------------------------------------------------------------");
	}
	Log::LogStack(root, db, m_espLen);
	
	const auto res = MessageBox(
		nullptr,
		"Application has stopped work!\n"
		"[Only for developers]: Do you want to perform intercept?",
		"Application Crash",
		MB_ICONERROR | MB_YESNO
	);
	return res == IDYES;
}
