#include "Process.h"

Process::Process()
	: m_forceTerminated(false) {
	notifyTerminated.connect([this]() {
		terminated(getNativeExitCode(), m_forceTerminated);
	});
}

Process::~Process() { }

void Process::terminate() {
	if(!isRunning()) {
		return;
	}

	m_forceTerminated = true;

	doTerminate();
}

bool Process::wasForceTerminated() const {
	return m_forceTerminated;
}

bool Process::didExitNormally() const {
	if(isRunning()) {
		return false;
	}

	return getNativeExitCode() == 0;
}
