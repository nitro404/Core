#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <boost/signals2.hpp>

#include <chrono>
#include <optional>
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
	void terminate();
	virtual uint64_t getNativeExitCode() const = 0;
	bool wasForceTerminated() const;
	bool didExitNormally() const;
	virtual std::optional<Priority> getPriority() const = 0;
virtual void getMemoryUsageInfo() const = 0;
virtual void getIOInfo() const = 0;
virtual void getProcessUptime() const = 0;

	boost::signals2::signal<void (uint64_t /* nativeExitCode */, bool /* forceTerminated */)> terminated;

	static constexpr Priority DEFAULT_PRIORITY = Priority::Normal;

protected:
	Process();

	virtual void doTerminate() = 0;

	boost::signals2::signal<void (void)> notifyTerminated;

private:
	std::atomic<bool> m_forceTerminated;
	boost::signals2::connection m_terminatedConnection;

	Process(const Process &) = delete;
	const Process & operator = (const Process &) = delete;
};

#endif // _PROCESS_H_
