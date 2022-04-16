#include "SegmentAnalyticsCURL.h"

using namespace std::chrono_literals;

SegmentAnalyticsCURL::AbstractFailedEvent::AbstractFailedEvent(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint)
	: m_retryTransferAfterTimePoint(retryTransferAfterTimePoint) { }

SegmentAnalyticsCURL::AbstractFailedEvent::AbstractFailedEvent(std::chrono::milliseconds retryTransferDelay)
	: m_retryTransferAfterTimePoint(std::chrono::system_clock::now() + retryTransferDelay) { }

SegmentAnalyticsCURL::AbstractFailedEvent::~AbstractFailedEvent() { }

bool SegmentAnalyticsCURL::AbstractFailedEvent::shouldRetryTransfer() const {
	return m_retryTransferAfterTimePoint <= std::chrono::system_clock::now();
}

std::chrono::time_point<std::chrono::system_clock> SegmentAnalyticsCURL::AbstractFailedEvent::getRetryTransferAfterTimePoint() const {
	return m_retryTransferAfterTimePoint;
}

std::chrono::milliseconds SegmentAnalyticsCURL::AbstractFailedEvent::getTimeUntilTransferRetry() const {
	if(m_retryTransferAfterTimePoint <= std::chrono::system_clock::now()) {
		return 0ms;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_retryTransferAfterTimePoint);
}

SegmentAnalyticsCURL::SingleFailedEvent::SingleFailedEvent(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint, std::shared_ptr<SegmentAnalyticEvent> analyticEvent)
	: AbstractFailedEvent(retryTransferAfterTimePoint)
	, m_analyticEvent(analyticEvent) { }

SegmentAnalyticsCURL::SingleFailedEvent::SingleFailedEvent(std::chrono::milliseconds retryTransferDelay, std::shared_ptr<SegmentAnalyticEvent> analyticEvent)
	: AbstractFailedEvent(retryTransferDelay)
	, m_analyticEvent(analyticEvent) { }

SegmentAnalyticsCURL::SingleFailedEvent::~SingleFailedEvent() { }

const std::shared_ptr<SegmentAnalyticEvent> SegmentAnalyticsCURL::SingleFailedEvent::getAnalyticEvent() const {
	return m_analyticEvent;
}

std::shared_ptr<SegmentAnalyticEvent> SegmentAnalyticsCURL::SingleFailedEvent::getAnalyticEvent() {
	return m_analyticEvent;
}

SegmentAnalyticsCURL::BatchFailedEvents::BatchFailedEvents(std::chrono::time_point<std::chrono::system_clock> retryTransferAfterTimePoint, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents)
	: AbstractFailedEvent(retryTransferAfterTimePoint)
	, m_analyticEvents(analyticEvents) { }

SegmentAnalyticsCURL::BatchFailedEvents::BatchFailedEvents(std::chrono::milliseconds retryTransferDelay, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents)
	: AbstractFailedEvent(retryTransferDelay)
	, m_analyticEvents(analyticEvents) { }

SegmentAnalyticsCURL::BatchFailedEvents::~BatchFailedEvents() { }

const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchFailedEvents::getAnalyticEvents() const {
	return m_analyticEvents;
}

std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchFailedEvents::getAnalyticEvents() {
	return m_analyticEvents;
}
