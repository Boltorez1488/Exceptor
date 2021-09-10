#pragma once
#include <macro.h>
#include <Poco/Logger.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <tchar.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")

#define MAX_NAME_LEN 256

// Return stack end
inline PTR PrintStack(Poco::Logger& root, CONTEXT* ctx, PTR& codeScrambler) {
	BOOL    result;
	HMODULE hModule;
	
	STACKFRAME64        stack;
	DWORD64             displacement;

	DWORD disp;
	IMAGEHLP_LINE64 *line;

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	char module[MAX_NAME_LEN];
	const auto pSymbol = (PSYMBOL_INFO)buffer;

	memset(&stack, 0, sizeof(STACKFRAME64));

	const HANDLE process = GetCurrentProcess();
	const HANDLE thread = GetCurrentThread();
	displacement = 0;
#if !X64
	stack.AddrPC.Offset = (*ctx).Eip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrStack.Offset = (*ctx).Esp;
	stack.AddrStack.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = (*ctx).Ebp;
	stack.AddrFrame.Mode = AddrModeFlat;
	const auto esp = ctx->Esp;
#else
	stack.AddrPC.Offset = (*ctx).Rip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrStack.Offset = (*ctx).Rsp;
	stack.AddrStack.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = (*ctx).Rbp;
	stack.AddrFrame.Mode = AddrModeFlat;
	const auto esp = ctx->Rsp;
#endif

	SymInitialize(process, nullptr, TRUE); //load symbols

	for (ULONG frame = 0; ; frame++) {
		// Get next call from stack
		result = StackWalk64(
			IS64(IMAGE_FILE_MACHINE_AMD64, IMAGE_FILE_MACHINE_I386),
			process,
			thread,
			&stack,
			ctx,
			nullptr,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			nullptr
		);

		if (!result) break;

		// Get symbol name for address
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		const auto symRes = SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, pSymbol);

		line = new IMAGEHLP_LINE64;
		line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		if (symRes) {
			hModule = nullptr;
			lstrcpyA(module, "");
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCTSTR)(stack.AddrPC.Offset), &hModule);
			if (hModule) {
				GetModuleFileNameA(hModule, module, MAX_NAME_LEN);
				if (!codeScrambler)
					codeScrambler = (PTR)stack.AddrPC.Offset;
			} else {
				root.fatal(IS64("| %016LX", "| %08lX"), (PTR)stack.AddrPC.Offset);
				delete line;
				line = nullptr;
				continue;
			}

			const auto modPath = Poco::replace(std::string(module), '\\', '/');
			Poco::StringTokenizer token(modPath, "/");
			const auto addr = (PTR)stack.AddrPC.Offset;
			const auto modOffset = addr - (PTR)hModule;
			const auto fOffset = addr - (PTR)pSymbol->Address;

			const auto stackPtr = (PTR)stack.AddrStack.Offset;
			const auto stackOffset = stackPtr - esp;

			root.fatal(
				IS64(
					"| %016LX - %s+%LX - %s+%LX [ESP: %LX][%LX+%LX]",
					"| %08lX - %s+%lX - %s+%lX [ESP: %08lX][%08lX+%lX]"
				),
				addr,
				*(token.end() - 1), modOffset,
				std::string(pSymbol->Name), fOffset,
				stackPtr,
				esp, stackOffset);
			if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &disp, line)) {
				root.fatal("| In: '%s' - Line: %lu", std::string(line->FileName), line->LineNumber);
				root.fatal("|");
			}
		} else {
			hModule = nullptr;
			lstrcpyA(module, "");
			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCTSTR)(stack.AddrPC.Offset), &hModule);
			if (hModule) {
				GetModuleFileNameA(hModule, module, MAX_NAME_LEN);
				if (!codeScrambler)
					codeScrambler = (PTR)stack.AddrPC.Offset;
			} else {
				root.fatal(IS64("| %016LX", "| %08lX"), (PTR)stack.AddrPC.Offset);
				delete line;
				line = nullptr;
				continue;
			}

			const auto modPath = Poco::replace(std::string(module), '\\', '/');
			Poco::StringTokenizer token(modPath, "/");
			const auto addr = (PTR)stack.AddrPC.Offset;
			const auto offset = addr - (PTR)hModule;

			const auto stackPtr = (PTR)stack.AddrStack.Offset;
			const auto stackOffset = stackPtr - esp;

			root.fatal(
				IS64(
					"| %016LX - %s+%LX [ESP: %LX][%LX+%LX]",
					"| %08lX - %s+%lX [ESP: %08lX][%08lX+%lX]"
				),
				addr,
				*(token.end() - 1), offset, 
				stackPtr, 
				esp, stackOffset);
		}

		delete line;
		line = nullptr;
	}
	return (PTR)stack.AddrStack.Offset;
}