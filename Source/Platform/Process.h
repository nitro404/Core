#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <chrono>
#include <string>

class Process {
public:
	enum class Priority {
		Idle,
		BelowNormal,
		Normal,
		AboveNormal,
		High,
		Realtime
	};

	virtual ~Process();

	virtual bool isRunning() const = 0;
	virtual void wait() = 0;
	virtual bool waitFor(std::chrono::milliseconds duration) = 0;
	virtual void terminate() = 0;
	virtual uint64_t getNativeExitCode() const = 0;
	virtual bool didExitNormally() const;

	static constexpr Priority DEFAULT_PRIORITY = Priority::Normal;

protected:
	Process();
};

#endif // _PROCESS_H_
