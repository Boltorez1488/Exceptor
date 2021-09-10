#include "Zip.h"
#include <sstream>

using namespace Data;

Zip::Zip(std::ostream& to, const Poco::DateTime& dt) : m_zip(to, false), m_dt(dt) {
}

Zip::~Zip() {
	m_zip.close();
}

void Zip::AddFile(const std::string& name, const std::string& data) {
	std::istringstream ss(data);
	m_zip.addFile(ss, m_dt, name);
}

void Zip::Close() {
	m_zip.close();
}
