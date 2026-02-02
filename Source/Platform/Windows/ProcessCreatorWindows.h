#ifndef _PROCESS_CREATOR_WINDOWS_H_
#define _PROCESS_CREATOR_WINDOWS_H_

#include "Platform/ProcessCreator.h"

class ProcessCreatorWindows : public ProcessCreator {
	friend class FactoryRegistry;

public:
	~ProcessCreatorWindows() override;

	virtual std::unique_ptr<Process> createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory = {}, Process::Priority = Process::DEFAULT_PRIORITY, uint64_t * nativeErrorCode = nullptr, std::string * nativeErrorMessage = nullptr) override;

private:
	ProcessCreatorWindows();

	ProcessCreatorWindows(const ProcessCreatorWindows &) = delete;
	const ProcessCreatorWindows & operator = (const ProcessCreatorWindows &) = delete;
};

#endif // _PROCESS_CREATOR_WINDOWS_H_
