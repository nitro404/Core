#include "SystemInformationBridgeWindows.h"

#include "Utilities/StringUtilities.h"

#include <windows.h>
#include <shlobj.h>

static std::optional<std::string> getKnownFolderPath(REFKNOWNFOLDERID knownFolderID) {
	PWSTR rawPath = nullptr;

	if(!SUCCEEDED(SHGetKnownFolderPath(knownFolderID, 0, nullptr, &rawPath))) {
		return {};
	}

	std::string path(Utilities::wideStringToString(rawPath));

	CoTaskMemFree(rawPath);

	return path;
}

SystemInformationBridgeWindows::SystemInformationBridgeWindows() { }

SystemInformationBridgeWindows::~SystemInformationBridgeWindows() { }

std::optional<std::string> SystemInformationBridgeWindows::getHomeDirectoryPath() {
	return getKnownFolderPath(FOLDERID_Profile);
}

std::optional<std::string> SystemInformationBridgeWindows::getApplicationDataDirectoryPath() {
	return getKnownFolderPath(FOLDERID_RoamingAppData);
}
