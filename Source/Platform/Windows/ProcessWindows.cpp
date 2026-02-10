#include "ProcessWindows.h"

#include "WindowsUtilities.h"

#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

ProcessWindows::ProcessWindows(HANDLE job, STARTUPINFO && startupInfo, PROCESS_INFORMATION && processInfo)
	: Process()
	, m_job(job)
	, m_startupInfo(startupInfo)
	, m_processInfo(processInfo)
	, m_running(true)
	, m_exitCode(0) {
	m_ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);

	JOBOBJECT_ASSOCIATE_COMPLETION_PORT jobAssociateCompletionPort;
	std::memset(&jobAssociateCompletionPort, 0, sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT));
	jobAssociateCompletionPort.CompletionKey = this;
	jobAssociateCompletionPort.CompletionPort = m_ioCompletionPort;

	SetInformationJobObject(m_job, JobObjectAssociateCompletionPortInformation, &jobAssociateCompletionPort, sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT));
}

ProcessWindows::~ProcessWindows() {
	doTerminate();
}

bool ProcessWindows::isRunning() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_running;
}

bool ProcessWindows::waitInternal(DWORD durationMs) {
	if(!m_running) {
		return false;
	}

	DWORD message = 0;
	ULONG_PTR completionKey = 0u;
	LPOVERLAPPED overlapped = nullptr;

	while(true) {
		if(!GetQueuedCompletionStatus(m_ioCompletionPort, &message, &completionKey, &overlapped, durationMs)) {
			if(overlapped == nullptr) {
				return false;
			}

			DWORD errorCode = GetLastError();

			switch(errorCode) {
				case ERROR_ABANDONED_WAIT_0: {
					cleanup();
					return true;
				}
				case ERROR_INVALID_HANDLE: {
					return true;
				}
				default: {
					spdlog::error("Failed to wait for job to complete with error: {}", WindowsUtilities::getLastErrorMessage());
					break;
				}
			}

			return false;
		}

		if(reinterpret_cast<ProcessWindows *>(completionKey) != this) {
			spdlog::error("Failed to wait for job to complete due to job associate completion key mismatch.");
			return false;
		}

		switch(message) {
			case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO: {
				cleanup();
				return true;
			}
			case JOB_OBJECT_MSG_END_OF_JOB_TIME:
			case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
			case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
			case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
			case JOB_OBJECT_MSG_EXIT_PROCESS:
			case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
			case JOB_OBJECT_MSG_NEW_PROCESS:
			case JOB_OBJECT_MSG_NOTIFICATION_LIMIT:
			case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT: {
				break;
			}
		}
	}
}

void ProcessWindows::wait() {
	waitInternal(INFINITE);
}

bool ProcessWindows::waitFor(std::chrono::milliseconds duration) {
	return waitInternal(static_cast<DWORD>(duration.count()));
}

void ProcessWindows::doTerminate() {
	if(m_running) {
		TerminateJobObject(m_job, 0);
	}

	cleanup();
}

void ProcessWindows::cleanup() {
	if(!m_running) {
		return;
	}

	m_running = false;

	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	DWORD exitCode = 0;
	GetExitCodeProcess(m_processInfo.hProcess, &exitCode);

	m_exitCode = static_cast<uint64_t>(exitCode);

	CloseHandle(m_processInfo.hProcess);
	CloseHandle(m_processInfo.hThread);
	CloseHandle(m_ioCompletionPort);
	CloseHandle(m_job);

	notifyTerminated();
}

uint64_t ProcessWindows::getNativeExitCode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_exitCode;
}

std::optional<Process::Priority> ProcessWindows::getPriority() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	DWORD priority = GetPriorityClass(m_processInfo.hProcess);

	if(priority == 0) {
		return {};
	}

	return getProcessPriority(priority);
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

Process::Priority ProcessWindows::getProcessPriority(DWORD priority) {
	switch(priority) {
		case IDLE_PRIORITY_CLASS: {
			return Priority::Idle;
		}
		case BELOW_NORMAL_PRIORITY_CLASS: {
			return Priority::BelowNormal;
		}
		case NORMAL_PRIORITY_CLASS: {
			return Priority::Normal;
		}
		case ABOVE_NORMAL_PRIORITY_CLASS: {
			return Priority::AboveNormal;
		}
		case HIGH_PRIORITY_CLASS: {
			return Priority::High;
		}
		case REALTIME_PRIORITY_CLASS: {
			return Priority::Realtime;
		}
	}

	return Priority::Normal;
}
