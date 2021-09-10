#include "OutLog.h"

void Debug::Log::LogMainInfo(Poco::Logger& root, PTR ptr, DWORD exCode, PCONTEXT db, const Poco::DateTime& dt) {
	const auto fmt = Poco::DateTimeFormatter::format(dt, "%d.%m.%Y %H:%M:%S");
	root.fatal("|-----------------------------------------------------------------------");
	root.fatal("| [%s]: Application Crash", fmt);
	root.fatal("|-----------------------------------------------------------------------");
	root.fatal("| Exception address: " IS64("%016LX", "%08lX"), ptr);
	root.fatal("| Code: %08lX", exCode);
	root.fatal("|-----------------------------------------------------------------------");
#if X64
	root.fatal("| RAX: %016LX", db->Rax);
	root.fatal("| RBX: %016LX", db->Rbx);
	root.fatal("| RCX: %016LX", db->Rcx);
	root.fatal("| RSI: %016LX", db->Rsi);
	root.fatal("| RDI: %016LX", db->Rdi);
	root.fatal("| RBP: %016LX", db->Rbp);
	root.fatal("| RSP: %016LX", db->Rsp);
#else
	root.fatal("| EAX: %08lX", db->Eax);
	root.fatal("| EBX: %08lX", db->Ebx);
	root.fatal("| ECX: %08lX", db->Ecx);
	root.fatal("| ESI: %08lX", db->Esi);
	root.fatal("| EDI: %08lX", db->Edi);
	root.fatal("| EBP: %08lX", db->Ebp);
	root.fatal("| ESP: %08lX", db->Esp);
#endif
}

PTR Debug::Log::LogCallStack(Poco::Logger& root, PTR ptr, PCONTEXT db, PTR& codePtr) {
	root.fatal("|-----------------------------------------------------------------------");
	root.fatal("| Call-Stack:");
	root.fatal("|-----------------------------------------------------------------------");
	CONTEXT ctx;
	ctx = *db;
	return PrintStack(root, &ctx, codePtr);
}

void Debug::Log::LogCode(Poco::Logger& root, PTR ptr, DWORD64 size) {
	ZydisDecoder decoder;
#if X64
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
#else
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);
#endif

	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	uint64_t iPtr = ptr;
	size_t offset = 0;
	ZydisDecodedInstruction instr;

	while (offset < size && ZydisDecoderDecodeBuffer(&decoder, (void*)(ptr + offset), 25, iPtr, &instr) == ZYDIS_STATUS_SUCCESS) {
		if (instr.opcode == 0xCC) {
			break;
		}

		char buffer[256];
		ZydisFormatterFormatInstruction(&formatter, &instr, buffer, sizeof buffer);
		std::string buff(buffer);
		const auto off = iPtr - ptr;
		if (iPtr == ptr)
			buff += " <==================================";
		if (iPtr == ptr)
			root.fatal("|  %04?X %s", (DWORD)off, buff);
		else
			root.fatal("| +%04?X %s", (DWORD)off, buff);

		offset += instr.length;
		iPtr += instr.length;
	}
}

void Debug::Log::LogStack(Poco::Logger& root, PCONTEXT db, size_t maxSize) {
	root.fatal("------------------------------------------------------------------------");
	root.fatal("- Dump " IS64("R", "E") "SP: ------------------------------------------------------------");
	root.fatal("------------------------------------------------------------------------");
	root.dump("", (void*)IS64(db->Rsp, db->Esp), maxSize, Poco::Message::PRIO_FATAL);
	root.fatal("------------------------------------------------------------------------");
}
