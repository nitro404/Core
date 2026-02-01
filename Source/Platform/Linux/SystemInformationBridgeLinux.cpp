#include "SystemInformationBridgeLinux.h"

#include "Utilities/FileUtilities.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>

SystemInformationBridgeLinux::SystemInformationBridgeLinux() { }

SystemInformationBridgeLinux::~SystemInformationBridgeLinux() { }

std::optional<std::string> SystemInformationBridgeLinux::getHomeDirectoryPath() {
	struct passwd * password = getpwuid(getuid());

	if(password) {
		return password->pw_dir;
	}

	const char * homeDirectoryPath = std::getenv("HOME");

	if(homeDirectoryPath) {
		return homeDirectoryPath;
	}

	return {};
}

std::optional<std::string> SystemInformationBridgeLinux::getApplicationDataDirectoryPath() {
	const char * xdgConfigHome = std::getenv("XDG_CONFIG_HOME");

	if(xdgConfigHome) {
		return xdgConfigHome;
	}

	std::optional<std::string> optionalHomeDirectoryPath(getHomeDirectoryPath());

	if(!optionalHomeDirectoryPath.has_value()) {
		return {};
	}

	return Utilities::joinPaths(optionalHomeDirectoryPath.value(), ".config");
}
