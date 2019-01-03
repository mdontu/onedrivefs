// SPDX-License-Identifier: GPL-2.0

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "appconfig.h"
#include "fuse.h"
#include "log.h"

using namespace OneDrive;

CAppConfig gConfig;
CLog gLog;

int main(int argc, const char *argv[])
{
	bool success = true;

	try {
		const char *home = getenv("HOME");

		if (!home)
			throw std::runtime_error("could not determine the user home directory");

		gConfig.setConfigDir(std::string(home) + "/.config/onedrivefs");
		gConfig.init();

		gLog.init("onedrivefs.log");

		CFuse fuse;

		if (fuse.init(argc, argv) < 0)
			success = false;
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		success = false;
	} catch (...) {
		std::cerr << "Error: an unknown exception has occurred" << std::endl;
		success = false;
	}

	return ( success ? 0 : 1 );
}
