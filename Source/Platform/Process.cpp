#include "Process.h"

Process::Process() { }

Process::~Process() { }

bool Process::didExitNormally() const {
	if(isRunning()) {
		return false;
	}

	return getNativeExitCode() == 0;
}
