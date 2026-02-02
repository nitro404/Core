#ifndef _PROCESS_CREATOR_LINUX_H_
#define _PROCESS_CREATOR_LINUX_H_

#include "Platform/ProcessCreator.h"

class ProcessCreatorLinux : public ProcessCreator {
	friend class FactoryRegistry;

public:
	~ProcessCreatorLinux() override;

	virtual std::unique_ptr<Process> createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory = {}, Process::Priority = Process::DEFAULT_PRIORITY, uint64_t * nativeErrorCode = nullptr, std::string * nativeErrorMessage = nullptr) override;

private:
	ProcessCreatorLinux();

	ProcessCreatorLinux(const ProcessCreatorLinux &) = delete;
	const ProcessCreatorLinux & operator = (const ProcessCreatorLinux &) = delete;
};

#endif // _PROCESS_CREATOR_LINUX_H_
