#ifndef _SEGMENT_ANALYTICS_CURL_H_
#define _SEGMENT_ANALYTICS_CURL_H_

#include "SegmentAnalytics.h"

#include <deque>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

class HTTPRequest;
class HTTPResponse;

class SegmentAnalyticsCURL final : public SegmentAnalytics,
                                   public SegmentAnalytics::LibraryInfoProvider {
public:
	SegmentAnalyticsCURL();
	virtual ~SegmentAnalyticsCURL();

	bool isRunning() const;

	// SegmentAnalytics Virtuals
	virtual bool initialize(const Configuration & configuration) override;
	virtual bool start() override;
	virtual void stop() override;
	virtual bool shouldTrackApplicationEvents() const override;
	virtual bool flush(std::chrono::milliseconds waitForDuration = std::chrono::milliseconds(0)) override;

	// LibraryInfoProvider Virtuals
	virtual const std::string & getLibraryName() const override;
	virtual const std::string & getLibraryVersion() const override;

protected:
	virtual const SegmentAnalytics::LibraryInfoProvider * getLibraryInfoProvider() const override;

	// SegmentAnalytics Virtuals
	virtual bool queueEvent(std::unique_ptr<SegmentAnalyticEvent> analyticEvent) override;

	bool sendSingleAnalyticEvent(std::shared_ptr<SegmentAnalyticEvent> analyticEvent);
	bool sendAnalyticEventBatch(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & events);

private:
	using AnalyticEventThread = std::unique_ptr<std::thread, std::function<void (std::thread *)>>;

	class AbstractEventTransfer {
	public:
		AbstractEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse);
		AbstractEventTransfer(AbstractEventTransfer && eventTransfer) noexcept;
		const AbstractEventTransfer & operator = (AbstractEventTransfer && eventTransfer) noexcept;
		virtual ~AbstractEventTransfer();

		const std::shared_ptr<HTTPRequest> getRequest() const;
		std::shared_ptr<HTTPRequest> getRequest();
		const std::future<std::shared_ptr<HTTPResponse>> & getFutureResponse() const;
		std::future<std::shared_ptr<HTTPResponse>> & getFutureResponse();

	private:
		std::shared_ptr<HTTPRequest> m_httpRequest;
		std::future<std::shared_ptr<HTTPResponse>> m_futureResponse;

		AbstractEventTransfer(const AbstractEventTransfer &) = delete;
		const AbstractEventTransfer & operator = (const AbstractEventTransfer &) = delete;
	};

	class SingleEventTransfer final : public AbstractEventTransfer {
	public:
		SingleEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse, std::shared_ptr<SegmentAnalyticEvent> analyticEvent);
		SingleEventTransfer(SingleEventTransfer && eventTransfer) noexcept;
		const SingleEventTransfer & operator = (SingleEventTransfer && eventTransfer) noexcept;
		virtual ~SingleEventTransfer();

		const std::shared_ptr<SegmentAnalyticEvent> getAnalyticEvent() const;
		std::shared_ptr<SegmentAnalyticEvent> getAnalyticEvent();

	private:
		std::shared_ptr<SegmentAnalyticEvent> m_analyticEvent;

		SingleEventTransfer(const SingleEventTransfer &) = delete;
		const SingleEventTransfer & operator = (const SingleEventTransfer &) = delete;
	};

	class BatchEventTransfer final : public AbstractEventTransfer {
	public:
		BatchEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents);
		BatchEventTransfer(BatchEventTransfer && eventTransfer) noexcept;
		const BatchEventTransfer & operator = (BatchEventTransfer && eventTransfer) noexcept;
		virtual ~BatchEventTransfer();

		const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & getAnalyticEvents() const;
		std::vector<std::shared_ptr<SegmentAnalyticEvent>> & getAnalyticEvents();

	private:
		std::vector<std::shared_ptr<SegmentAnalyticEvent>> m_analyticEvents;

		BatchEventTransfer(const BatchEventTransfer &) = delete;
		const BatchEventTransfer & operator = (const BatchEventTransfer &) = delete;
	};

	class AbstractFailedEvent {
	public:
		AbstractFailedEvent(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint);
		AbstractFailedEvent(std::chrono::milliseconds retryTransferDelay);
		AbstractFailedEvent(AbstractFailedEvent && failedEvent) noexcept;
		const AbstractFailedEvent & operator = (AbstractFailedEvent && failedEvent) noexcept;
		virtual ~AbstractFailedEvent();

		bool shouldRetryTransfer() const;
		std::chrono::time_point<std::chrono::system_clock> getRetryTransferAfterTimePoint() const;
		std::chrono::milliseconds getTimeUntilTransferRetry() const;

	private:
		std::chrono::time_point<std::chrono::system_clock> m_retryTransferAfterTimePoint;

		AbstractFailedEvent(const AbstractFailedEvent &) = delete;
		const AbstractFailedEvent & operator = (const AbstractFailedEvent &) = delete;
	};

	class SingleFailedEvent final : public AbstractFailedEvent {
	public:
		SingleFailedEvent(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint, std::shared_ptr<SegmentAnalyticEvent> analyticEvent);
		SingleFailedEvent(std::chrono::milliseconds retryTransferDelay, std::shared_ptr<SegmentAnalyticEvent> analyticEvent);
		SingleFailedEvent(SingleFailedEvent && failedEvent) noexcept;
		const SingleFailedEvent & operator = (SingleFailedEvent && failedEvent) noexcept;
		virtual ~SingleFailedEvent();

		const std::shared_ptr<SegmentAnalyticEvent> getAnalyticEvent() const;
		std::shared_ptr<SegmentAnalyticEvent> getAnalyticEvent();

	private:
		std::shared_ptr<SegmentAnalyticEvent> m_analyticEvent;

		SingleFailedEvent(const SingleFailedEvent &) = delete;
		const SingleFailedEvent & operator = (const SingleFailedEvent &) = delete;
	};

	class BatchFailedEvents final : public AbstractFailedEvent {
	public:
		BatchFailedEvents(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents);
		BatchFailedEvents(std::chrono::milliseconds retryTransferDelay, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents);
		BatchFailedEvents(BatchFailedEvents && failedEvent) noexcept;
		const BatchFailedEvents & operator = (BatchFailedEvents && failedEvent) noexcept;
		virtual ~BatchFailedEvents();

		const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & getAnalyticEvents() const;
		std::vector<std::shared_ptr<SegmentAnalyticEvent>> & getAnalyticEvents();

	private:
		std::vector<std::shared_ptr<SegmentAnalyticEvent>> m_analyticEvents;

		BatchFailedEvents(const BatchFailedEvents &) = delete;
		const BatchFailedEvents & operator = (const BatchFailedEvents &) = delete;
	};

	std::shared_ptr<HTTPRequest> createSingleAnalyticEventRequest(const SegmentAnalyticEvent & analyticEvent);
	std::shared_ptr<HTTPRequest> createBatchAnalyticEventRequest(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents);
	void configureAnalyticEventRequest(HTTPRequest & request);
	void run();

	static const std::string DEFAULT_API_ADDRESS;

	bool m_running;
	bool m_flushRequested;
	bool m_stopRequested;
	std::string m_apiAddress;
	std::chrono::milliseconds m_failedNetworkTransferRetryDelay;
	std::deque<std::shared_ptr<SegmentAnalyticEvent>> m_queuedEvents;
	std::map<uint64_t, std::unique_ptr<AbstractEventTransfer>> m_analyticEventTransfers;
	std::vector<std::unique_ptr<AbstractFailedEvent>> m_failedEvents;
	AnalyticEventThread m_analyticEventThread;
	mutable std::recursive_mutex m_flushMutex;
	mutable std::condition_variable_any m_flushWaitCondition;

	SegmentAnalyticsCURL(const SegmentAnalyticsCURL &) = delete;
	const SegmentAnalyticsCURL & operator = (const SegmentAnalyticsCURL &) = delete;
};

#endif // _SEGMENT_ANALYTICS_CURL_H_
