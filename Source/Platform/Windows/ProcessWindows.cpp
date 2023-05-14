#include "ProcessWindows.h"

void CALLBACK onProcessExited(LPVOID context, BOOLEAN timedOut) {
	reinterpret_cast<ProcessWindows *>(context)->onProcessTerminated(timedOut);
}

ProcessWindows::ProcessWindows(const STARTUPINFO & startupInfo, const PROCESS_INFORMATION & processInfo)
	: Process()
	, m_startupInfo(startupInfo)
	, m_processInfo(processInfo)
	, m_running(true)
	, m_exitCode(0) {
	RegisterWaitForSingleObject(&m_waitHandle, m_processInfo.hProcess, onProcessExited, this, INFINITE, WT_EXECUTEONLYONCE);
}

ProcessWindows::~ProcessWindows() {
	UnregisterWaitEx(m_waitHandle, INVALID_HANDLE_VALUE);

	if(m_running) {
		TerminateProcess(m_processInfo.hProcess, 0);
	}

	cleanup();
}

void ProcessWindows::onProcessTerminated(bool timedOut) {
	cleanup();
}

bool ProcessWindows::isRunning() const {
	return m_running;
}

void ProcessWindows::wait() {
	if(!m_running) {
		return;
	}

	WaitForSingleObject(m_processInfo.hProcess, INFINITE);

	cleanup();
}

bool ProcessWindows::waitFor(std::chrono::milliseconds duration) {
	DWORD waitResult = WaitForSingleObject(m_processInfo.hProcess, duration.count());

	switch(waitResult) {
		case WAIT_OBJECT_0: {
			cleanup();

			return true;
		}

		case WAIT_ABANDONED:
		case WAIT_TIMEOUT:
		case WAIT_FAILED: {
			break;
		}
	}

	return false;
}

void ProcessWindows::doTerminate() {
	if(m_running) {
		TerminateProcess(m_processInfo.hProcess, 0);
	}

	cleanup();
}

void ProcessWindows::cleanup() {
	if(!m_running) {
		return;
	}

	m_running = false;

	DWORD exitCode;
	GetExitCodeProcess(m_processInfo.hProcess, &exitCode);

	m_exitCode = static_cast<uint64_t>(exitCode);

	CloseHandle(m_processInfo.hProcess);
	CloseHandle(m_processInfo.hThread);

	notifyTerminated();
}

uint64_t ProcessWindows::getNativeExitCode() const {
	return m_exitCode;
}

DWORD ProcessWindows::getWindowsProcessPriority(Process::Priority priority) {
	switch(priority) {
		case Priority::Idle: {
			return IDLE_PRIORITY_CLASS;
		}
		case Priority::BelowNormal: {
			return BELOW_NORMAL_PRIORITY_CLASS;
		}
		case Priority::Normal: {
			return NORMAL_PRIORITY_CLASS;
		}
		case Priority::AboveNormal: {
			return ABOVE_NORMAL_PRIORITY_CLASS;
		}
		case Priority::High: {
			return HIGH_PRIORITY_CLASS;
		}
		case Priority::Realtime: {
			return REALTIME_PRIORITY_CLASS;
		}
	}

	return NORMAL_PRIORITY_CLASS;
}
