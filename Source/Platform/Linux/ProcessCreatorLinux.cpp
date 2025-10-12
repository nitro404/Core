#include "ProcessCreatorLinux.h"

ProcessCreatorLinux::ProcessCreatorLinux()
	: ProcessCreator() { }

ProcessCreatorLinux::~ProcessCreatorLinux() { }

std::unique_ptr<Process> ProcessCreatorLinux::createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory, Process::Priority priority, uint64_t * nativeErrorCode, std::string * nativeErrorMessage) {
	// Note: This functionality is not yet implemented.
	return nullptr;
}
