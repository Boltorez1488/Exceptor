#include "header.h"
#include <Poco/ConsoleChannel.h>
#include <Poco/Checksum.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Formatter.h>
#include <cryptopp/crc.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <StaticLibs/CryptoPPLibs.h>

using namespace Poco;
using namespace CryptoPP;

void test() {
	FileInputStream fs("D3Launcher.exe");
	std::string data;
	StreamCopier::copyToString(fs, data);
	//FileSource("D3Launcher.exe", true, new StringSink(data));
	Checksum sum;
	sum.update(data);
	std::string res = format("%?X", sum.checksum());

	CRC32 crc;
	std::string data2;
	FileSource("D3Launcher.exe", true, new HashFilter(crc, new StringSink(data2)));
	const auto sum2 = *(UINT32*)data2.data();
	const auto res2 = format("%?X", sum2);

	const auto out = format("%s / %s", res, res2);
	MessageBox(0, out.c_str(), 0, 0);
	/*const auto ptr = (DWORD)0x222;

	AutoPtr<Poco::ConsoleChannel> pChannel = new Poco::ConsoleChannel;
	Poco::Logger& root = Poco::Logger::root();
	root.setChannel(pChannel);
	root.error("Application crashed");
	root.error("Exception address: %08lx", ptr);*/
	//*(DWORD*)0x0 = 123;
	//*(DWORD*)0x0 = 123;
}
