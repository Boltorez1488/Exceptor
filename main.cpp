#include "header.h"
#include "Debug/Exceptor.h"
#include <Poco/FileStream.h>

extern void test();

class SampleApp : public Application {
	bool _helpRequested;
public:
	SampleApp() : _helpRequested(false) {
	}

protected:
	void initialize(Application& self) override {
		loadConfiguration();
		Application::initialize(self);
	}

	void uninitialize() override {
		Application::uninitialize();
	}

	void reinitialize(Application& self) override {
		Application::reinitialize(self);
	}

	void defineOptions(OptionSet& options) override {
		Application::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<SampleApp>(this, &SampleApp::handleHelp)));

		options.addOption(
			Option("define", "D", "define a configuration property")
			.required(false)
			.repeatable(true)
			.argument("name=value")
			.callback(OptionCallback<SampleApp>(this, &SampleApp::handleDefine)));

		options.addOption(
			Option("config-file", "f", "load configuration data from a file")
			.required(false)
			.repeatable(true)
			.argument("file")
			.callback(OptionCallback<SampleApp>(this, &SampleApp::handleConfig)));

		options.addOption(
			Option("bind", "b", "bind option value to test.property")
			.required(false)
			.repeatable(false)
			.argument("value")
			.binding("test.property"));
	}

	void handleHelp(const std::string& name, const std::string& value) {
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	// ReSharper disable once CppMemberFunctionMayBeConst
	void handleDefine(const std::string& name, const std::string& value) {
		defineProperty(value);
	}

	void handleConfig(const std::string& name, const std::string& value) {
		loadConfiguration(value);
	}

	void displayHelp() const {
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("Test application for power SEH exception hooker");
		helpFormatter.format(std::cout);
	}

	void defineProperty(const std::string& def) const {
		std::string name;
		std::string value;
		std::string::size_type pos = def.find('=');
		if (pos != std::string::npos) {
			name.assign(def, 0, pos);
			value.assign(def, pos + 1, def.length() - pos);
		} else name = def;
		config().setString(name, value);
	}

	int main(const ArgVec& args) override {
		if (!_helpRequested) {
			/*Poco::File f("debug.txt");
			if (f.exists())
				f.remove();
			Debug::Exceptor::Attach(new Poco::FileChannel("debug.txt"));*/
			Debug::Exceptor::AttachFolder();
			/*Debug::Exceptor::AttachCallbackZip([](const std::string& data) {
				Poco::FileOutputStream fs("debug.zip");
				fs << data;
				return false;
			});*/
			test();
		}
		return EXIT_OK;
	}
};


POCO_APP_MAIN(SampleApp)