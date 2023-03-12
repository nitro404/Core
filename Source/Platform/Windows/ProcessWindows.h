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
	virtual ~ProcessWindows();

	void onProcessTerminated(bool timedOut);

	// Process Virtuals
	virtual bool isRunning() const override;
	virtual void wait() override;
	virtual bool waitFor(std::chrono::milliseconds duration) override;
	virtual void doTerminate() override;
	virtual uint64_t getNativeExitCode() const override;

	static DWORD getWindowsProcessPriority(Process::Priority priority);

private:
	ProcessWindows(const STARTUPINFO & startupInfo, const PROCESS_INFORMATION & processInfo);

	void cleanup();

	STARTUPINFO m_startupInfo;
	PROCESS_INFORMATION m_processInfo;
	HANDLE m_waitHandle;
	std::atomic<bool> m_running;
	std::atomic<uint64_t> m_exitCode;
};

#endif // _PROCESS_WINDOWS_H_
