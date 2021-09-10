#include "CallbackLogger.h"
#include "../../Data/Zip.h"

using namespace Debug::Loggers;

CallbackLogger::CallbackLogger(bool(*callback)(const std::string&), size_t espLen) :
m_callback(callback), m_espLen(espLen) {
}

CallbackLogger::~CallbackLogger() = default;

bool CallbackLogger::Log(PTR ptr, DWORD exCode, PCONTEXT db) {
	Poco::Logger& root = Poco::Logger::root();
	std::ostringstream outMain;
	const auto pChannel = new Poco::StreamChannel(outMain);
	root.setChannel(pChannel);

	const auto dt = Poco::DateTime();
	std::ostringstream zs;
	Data::Zip zip(zs, dt);
	Log::LogMainInfo(root, ptr, exCode, db, dt);

	PTR codePtr = NULL;
	const auto stackEnd = Log::LogCallStack(root, ptr, db, codePtr);
	const auto stackLen = stackEnd - IS64(db->Rsp, db->Esp);
	if (m_espLen > stackLen)
		m_espLen = stackLen;
	if (!IsBadCodePtr((FARPROC)codePtr)) {
		const auto pStart = codePtr - CODE_BLOCK_SIZE / 2;
		root.fatal("|-----------------------------------------------------------------------");
		root.fatal(IS64("| Ptr-Asm: %016LX", "| Ptr-Asm: %08lX"), codePtr);
		root.fatal(IS64("| Zip-Ptr-Start: %016?X", "| Zip-Ptr-Start: %08?X"), pStart);
		root.fatal("| Zip-Center: %?X", CODE_BLOCK_SIZE / 2);
		std::ostringstream ss;
		ss.write((char*)pStart, CODE_BLOCK_SIZE);
		zip.AddFile("code.bin", ss.str());

		root.fatal("|-----------------------------------------------------------------------");
		Log::LogCode(root, codePtr, 256);
	} else {
		root.fatal("|-----------------------------------------------------------------------");
		root.fatal("| Bad executable pointer");
		root.fatal("|-----------------------------------------------------------------------");
	}

	Log::LogStack(root, db, m_espLen);
	zip.AddFile("crash.log", outMain.str());

	std::ostringstream espStream;
	espStream.write((char*)IS64(db->Rsp, db->Esp), stackLen);
	zip.AddFile("stack.bin", espStream.str());

	zip.Close();
	if(m_callback) {
		return m_callback(zs.str());
	}
	return false;
}
