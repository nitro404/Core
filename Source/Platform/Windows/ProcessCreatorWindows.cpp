#include "ProcessCreatorWindows.h"

#include "ProcessWindows.h"
#include "Utilities/FileUtilities.h"
#include "WindowsUtilities.h"

#include <spdlog/spdlog.h>

#include <filesystem>

ProcessCreatorWindows::ProcessCreatorWindows()
	: ProcessCreator() { }

ProcessCreatorWindows::~ProcessCreatorWindows() { }

std::unique_ptr<Process> ProcessCreatorWindows::createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory, Process::Priority priority, uint64_t * nativeErrorCode, std::string * nativeErrorMessage) {
	HANDLE job = CreateJobObject(nullptr, nullptr);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo;
	std::memset(&jobInfo, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
	jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));

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
		AssignProcessToJobObject(job, processInfo.hProcess);

		return std::unique_ptr<Process>(new ProcessWindows(job, std::move(startupInfo), std::move(processInfo)));
	}

	DWORD errorCode = GetLastError();
	std::string errorMessage(WindowsUtilities::getLastErrorMessage());

	if(nativeErrorCode != nullptr) {
		*nativeErrorCode = static_cast<uint64_t>(GetLastError());
	}

	if(nativeErrorMessage != nullptr) {
		*nativeErrorMessage = errorMessage;
	}

	spdlog::error("Failed to create process with error: {}", errorMessage);

	return nullptr;
}
