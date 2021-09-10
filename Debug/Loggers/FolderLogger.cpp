#include "FolderLogger.h"
#include <utility>
#include "../../Data/Zip.h"

using namespace Debug::Loggers;

FolderLogger::FolderLogger(std::string outDir, size_t espLen) : m_outDir(std::move(outDir)), m_espLen(espLen) {

}

FolderLogger::~FolderLogger() = default;

bool FolderLogger::Log(PTR ptr, DWORD exCode, PCONTEXT db) {
	Poco::Logger& root = Poco::Logger::root();
	std::ostringstream outMain;
	const auto pChannel = new Poco::StreamChannel(outMain);
	root.setChannel(pChannel);
	
	const auto dt = Poco::DateTime();
	Poco::File d(m_outDir);
	if (!d.exists())
		d.createDirectories();
	Poco::Path p(m_outDir);
	p.append(Poco::DateTimeFormatter::format(dt, "%d.%m.%Y-%H.%M.%S") + "_crash.zip");
	const auto path = p.toString();
	Poco::FileOutputStream fs(path);
	Data::Zip zip(fs, dt);
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
	fs.close();

	const auto res = MessageBox(
		nullptr,
		Poco::format(
			"Application has stopped work!\n"
			"Report saved to \"%s\".\n"
			"[Only for developers]: Do you want to perform intercept?",
			path
		).c_str(),
		"Application Crash",
		MB_ICONERROR | MB_YESNO
	);
	return res == IDYES;
}
