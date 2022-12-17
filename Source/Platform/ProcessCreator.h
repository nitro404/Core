#ifndef _PROCESS_CREATOR_H_
#define _PROCESS_CREATOR_H_

#include "Process.h"
#include "Singleton/Singleton.h"

#include <memory>
#include <optional>
#include <string>

class ProcessCreator : public Singleton<ProcessCreator> {
public:
	virtual ~ProcessCreator();

	virtual std::unique_ptr<Process> createProcess(const std::string & applicationCommand, const std::optional<std::string> & workingDirectory = {}, Process::Priority priority = Process::DEFAULT_PRIORITY, uint64_t * nativeErrorCode = nullptr, std::string * nativeErrorMessage = nullptr) = 0;

protected:
	ProcessCreator();

private:
	ProcessCreator(const ProcessCreator &) = delete;
	ProcessCreator(ProcessCreator &&) noexcept = delete;
	const ProcessCreator & operator = (const ProcessCreator &) = delete;
	const ProcessCreator & operator = (ProcessCreator &&) noexcept = delete;
};

#endif // _PROCESS_CREATOR_H_
