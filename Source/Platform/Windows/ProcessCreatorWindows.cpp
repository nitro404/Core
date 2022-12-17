#include "ProcessCreatorWindows.h"

#include "ProcessWindows.h"
#include "Utilities/FileUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

ProcessCreatorWindows::ProcessCreatorWindows()
	: ProcessCreator() { }

ProcessCreatorWindows::~ProcessCreatorWindows() { }

std::unique_ptr<Process> ProcessCreatorWindows::createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory, Process::Priority priority, uint64_t * nativeErrorCode, std::string * nativeErrorMessage) {
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	std::memset(&startupInfo, 0, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	std::memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));

	std::string finalWorkingDirectory(workingDirectory.value_or("").empty() ? std::filesystem::current_path().string() : workingDirectory.value());

	if(CreateProcess(
		nullptr, // module name
		const_cast<char *>(applicationCommand.c_str()), // command line
		nullptr, // process handle inheritance
		nullptr, // thread handle inheritance
		false, // handle inheritance
		ProcessWindows::getWindowsProcessPriority(priority), // process creation flags
		nullptr, // environment block
		workingDirectory.has_value() ? finalWorkingDirectory.c_str() : nullptr, // starting directory
		&startupInfo, // process startup information
		&processInfo // process information
	)) {
		return std::unique_ptr<Process>(new ProcessWindows(startupInfo, processInfo));
	}

	DWORD errorCode = GetLastError();
	std::string errorMessage;

	if(nativeErrorCode != nullptr) {
		*nativeErrorCode = static_cast<uint64_t>(errorCode);
	}

	if (errorCode) {
		LPVOID errorMessageBuffer = nullptr;

		DWORD errorMessageLength = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // formatting options
			nullptr, // message definition location
			errorCode, // message identifier
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // language identifier
			reinterpret_cast<LPTSTR>(&errorMessageBuffer), // buffer
			0, // output buffer size
			nullptr // message arguments
		);

		if(errorMessageLength != 0) {
			LPCSTR errorMessageString = reinterpret_cast<LPCSTR>(errorMessageBuffer);
			errorMessage = std::string(errorMessageString, errorMessageString + errorMessageLength);

			LocalFree(errorMessageBuffer);
		}
	}

	if(nativeErrorMessage != nullptr) {
		*nativeErrorMessage = errorMessage;
	}

	spdlog::error("Failed to create process: {}", errorMessage);

	return nullptr;
}
