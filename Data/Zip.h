#pragma once
#include <Poco/Zip/Compress.h>
#include <Poco/DateTime.h>

namespace Data {
	class Zip {
		Poco::Zip::Compress m_zip;
		Poco::DateTime m_dt;
	public:
		Zip(std::ostream& to, const Poco::DateTime& dt);
		~Zip();

		void AddFile(const std::string& name, const std::string& data);

		void Close();
	};
}