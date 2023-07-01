#include "ProcessWindows.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/Windows/TimeUtilitiesWindows.h"

#include <spdlog/spdlog.h>

#include <Psapi.h>
#include <WinBase.h>

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

std::optional<Process::Priority> ProcessWindows::getPriority() const {
	DWORD priority = GetPriorityClass(m_processInfo.hProcess);

	if(priority == 0) {
		return {};
	}

	return getProcessPriority(priority);
}

void ProcessWindows::getMemoryUsageInfo() const {
	PROCESS_MEMORY_COUNTERS processMemoryCounters;

	if(!GetProcessMemoryInfo(m_processInfo.hProcess, &processMemoryCounters, sizeof(processMemoryCounters))) {
		return;
	}

spdlog::info("PageFaultCount: {}", processMemoryCounters.PageFaultCount);
spdlog::info("WorkingSetSize: {}", Utilities::fileSizeToString(processMemoryCounters.WorkingSetSize));
spdlog::info("QuotaPagedPoolUsage: {}", Utilities::fileSizeToString(processMemoryCounters.QuotaPagedPoolUsage));
spdlog::info("QuotaNonPagedPoolUsage: {}", Utilities::fileSizeToString(processMemoryCounters.QuotaNonPagedPoolUsage));
spdlog::info("PagefileUsage: {}", Utilities::fileSizeToString(processMemoryCounters.PagefileUsage));
}

void ProcessWindows::getIOInfo() const {
	IO_COUNTERS processIOCounters;

	if(!GetProcessIoCounters(m_processInfo.hProcess, &processIOCounters)) {
		return;
	}

spdlog::info("ReadOperationCount: {}", processIOCounters.ReadOperationCount);
spdlog::info("WriteOperationCount: {}", processIOCounters.WriteOperationCount);
spdlog::info("OtherOperationCount: {}", processIOCounters.OtherOperationCount);
spdlog::info("ReadTransferCount: {}", Utilities::fileSizeToString(processIOCounters.ReadTransferCount));
spdlog::info("WriteTransferCount: {}", Utilities::fileSizeToString(processIOCounters.WriteTransferCount));
spdlog::info("OtherTransferCount: {}", Utilities::fileSizeToString(processIOCounters.OtherTransferCount));
}

void ProcessWindows::getProcessUptime() const {
	FILETIME processCreationFileTime;
	FILETIME processExitFileTime;
	FILETIME kernelFileTime;
	FILETIME userFileTime;

	if(!GetProcessTimes(m_processInfo.hProcess, &processCreationFileTime, &processExitFileTime, &kernelFileTime, &userFileTime)) {
		return;
	}

	std::chrono::time_point<std::chrono::system_clock> processCreationTime = Utilities::fileTimeToSystemClockTime(processCreationFileTime);
	std::chrono::milliseconds kernelTime = Utilities::fileTimeToDuration(kernelFileTime);
	std::chrono::milliseconds userTime = Utilities::fileTimeToDuration(userFileTime);

spdlog::info("ProcessCreationTime: {}", Utilities::timePointToString(processCreationTime, Utilities::TimeFormat::ISO8601));
spdlog::info("KernelTime: {} ms", kernelTime.count());
spdlog::info("UserTime: {} ms", userTime.count());
}

/*
// TODO: hmm.. none of this seems valid? not sure.:
PageFaultCount: 23816
WorkingSetSize: 82120704
QuotaPagedPoolUsage: 343448
QuotaNonPagedPoolUsage: 29016
PagefileUsage: 122527744
ReadOperationCount: 42477
WriteOperationCount: 5
OtherOperationCount: 1643
ReadTransferCount: 5108387
WriteTransferCount: 683
OtherTransferCount: 355368
ProcessCreationTime: 2023-07-01T05:56:26.3710000Z
ProcessExitTime: 1601-01-01T00:00:00.0000000Z
// TODO: this looks like some sort of cumulate time represented in a file time object? Maybe just count it?:
KernelTime: 1601-01-01T00:00:00.3740000Z
UserTime: 1601-01-01T00:00:03.3690000Z
// TODO: what about CPU usage? maybe not relevant.
// TODO: what about process priority?
*/

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
