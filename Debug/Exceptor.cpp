#include "Exceptor.h"
#include "Loggers/ChannelLogger.h"
#include "Loggers/FolderLogger.h"
#include "Loggers/CallbackLogger.h"
#include <csignal>
#include <cstdlib>

using namespace Debug;
static Loggers::ILogger* _logger = nullptr;

static LONG WINAPI hooker(EXCEPTION_POINTERS* ep) {
	switch(ep->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_SINGLE_STEP:
		case EXCEPTION_BREAKPOINT:
			return EXCEPTION_CONTINUE_SEARCH;
		default:
			break;
	}
	static bool locked = false;
	if (locked)
		return EXCEPTION_CONTINUE_SEARCH;

	locked = true;
	const auto ptr = (PTR)ep->ExceptionRecord->ExceptionAddress;
	const auto db = ep->ContextRecord;

	if (!_logger->Log(ptr, ep->ExceptionRecord->ExceptionCode, db)) {
		locked = false;
		std::exit(0x0);
	}

	locked = false;
	return EXCEPTION_CONTINUE_EXECUTION;
}

static void catchSignal(int sig) {
	delete _logger;
}

static void init() {
	std::atexit([]() {
		delete _logger;
	});
	signal(SIGINT, catchSignal); // Interrupt
	signal(SIGABRT, catchSignal); // abort()
	signal(SIGTERM, catchSignal); // sent by "kill" command
}

void Exceptor::Attach(Poco::Channel* pChannel, size_t espLen) {
	if (_logger)
		return;
	init();
	_logger = new Loggers::ChannelLogger(pChannel, espLen);
	AddVectoredExceptionHandler(0, hooker);
}

void Exceptor::AttachFolder(const std::string& dir, size_t espLen) {
	if (_logger)
		return;
	init();
	_logger = new Loggers::FolderLogger(dir, espLen);
	AddVectoredExceptionHandler(0, hooker);
}

void Exceptor::AttachCallbackZip(bool(*callback)(const std::string&), size_t espLen) {
	if (_logger)
		return;
	init();
	_logger = new Loggers::CallbackLogger(callback, espLen);
	AddVectoredExceptionHandler(0, hooker);
}
