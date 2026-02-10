#include "Process.h"

#include <spdlog/spdlog.h>

Process::Process()
	: m_forceTerminated(false)
	, m_terminatedConnection(notifyTerminated.connect([this]() {
		terminated(getNativeExitCode(), m_forceTerminated);
	})) { }

Process::~Process() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_terminatedConnection.disconnect();
}

void Process::terminate() {
	if(!isRunning()) {
		return;
	}

	m_forceTerminated = true;

	doTerminate();
}

bool Process::wasForceTerminated() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_forceTerminated;
}

bool Process::didExitNormally() const {
	if(isRunning()) {
		return false;
	}

	return getNativeExitCode() == 0;
}
