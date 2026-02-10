#ifndef _PROCESS_WINDOWS_H_
#define _PROCESS_WINDOWS_H_

#include "Platform/Process.h"

#include <windows.h>
#include <processthreadsapi.h>

#include <cstdint>

class ProcessCreatorWindows;

class ProcessWindows final : public Process {
	friend class ProcessCreatorWindows;

public:
	~ProcessWindows() override;

	// Process Virtuals
	virtual bool isRunning() const override;
	virtual void wait() override;
	virtual bool waitFor(std::chrono::milliseconds duration) override;
	virtual void doTerminate() override;
	virtual uint64_t getNativeExitCode() const override;
	virtual std::optional<Priority> getPriority() const override;

	static DWORD getWindowsProcessPriority(Process::Priority priority);
	static Process::Priority getProcessPriority(DWORD priority);

private:
	ProcessWindows(HANDLE job, STARTUPINFO && startupInfo, PROCESS_INFORMATION && processInfo);

	bool waitInternal(DWORD durationMs = INFINITE);
	void cleanup();

	HANDLE m_job;
	HANDLE m_ioCompletionPort;
	STARTUPINFO m_startupInfo;
	PROCESS_INFORMATION m_processInfo;
	std::atomic<bool> m_running;
	std::atomic<uint64_t> m_exitCode;

	ProcessWindows(const ProcessWindows &) = delete;
	const ProcessWindows & operator = (const ProcessWindows &) = delete;
};

#endif // _PROCESS_WINDOWS_H_
