#include "SegmentAnalyticsCURL.h"

#include <Network/HTTPRequest.h>

SegmentAnalyticsCURL::AbstractEventTransfer::AbstractEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse)
	: m_httpRequest(httpRequest)
	, m_futureResponse(std::move(futureResponse)) { }

SegmentAnalyticsCURL::AbstractEventTransfer::AbstractEventTransfer(AbstractEventTransfer && eventTransfer) noexcept
	: m_httpRequest(std::move(eventTransfer.m_httpRequest))
	, m_futureResponse(std::move(eventTransfer.m_futureResponse)) { }

const SegmentAnalyticsCURL::AbstractEventTransfer & SegmentAnalyticsCURL::AbstractEventTransfer::operator = (AbstractEventTransfer && eventTransfer) noexcept {
	if(this != &eventTransfer) {
		m_httpRequest = std::move(eventTransfer.m_httpRequest);
		m_futureResponse = std::move(eventTransfer.m_futureResponse);
	}

	return *this;
}

SegmentAnalyticsCURL::AbstractEventTransfer::~AbstractEventTransfer() { }

const std::shared_ptr<HTTPRequest> SegmentAnalyticsCURL::AbstractEventTransfer::getRequest() const {
	return m_httpRequest;
}

std::shared_ptr<HTTPRequest> SegmentAnalyticsCURL::AbstractEventTransfer::getRequest() {
	return m_httpRequest;
}

const std::future<std::shared_ptr<HTTPResponse>> & SegmentAnalyticsCURL::AbstractEventTransfer::getFutureResponse() const {
	return m_futureResponse;
}

std::future<std::shared_ptr<HTTPResponse>> & SegmentAnalyticsCURL::AbstractEventTransfer::getFutureResponse() {
	return m_futureResponse;
}

SegmentAnalyticsCURL::SingleEventTransfer::SingleEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse, std::shared_ptr<SegmentAnalyticEvent> analyticEvent)
	: AbstractEventTransfer(httpRequest, std::move(futureResponse))
	, m_analyticEvent(analyticEvent) { }

SegmentAnalyticsCURL::SingleEventTransfer::SingleEventTransfer(SingleEventTransfer && eventTransfer) noexcept
	: AbstractEventTransfer(std::move(eventTransfer))
	, m_analyticEvent(std::move(eventTransfer.m_analyticEvent)) { }

const SegmentAnalyticsCURL::SingleEventTransfer & SegmentAnalyticsCURL::SingleEventTransfer::operator = (SingleEventTransfer && eventTransfer) noexcept {
	if(this != &eventTransfer) {
		AbstractEventTransfer::operator = (std::move(eventTransfer));

		m_analyticEvent = std::move(eventTransfer.m_analyticEvent);
	}

	return *this;
}

SegmentAnalyticsCURL::SingleEventTransfer::~SingleEventTransfer() { }

const std::shared_ptr<SegmentAnalyticEvent> SegmentAnalyticsCURL::SingleEventTransfer::getAnalyticEvent() const {
	return m_analyticEvent;
}

std::shared_ptr<SegmentAnalyticEvent> SegmentAnalyticsCURL::SingleEventTransfer::getAnalyticEvent() {
	return m_analyticEvent;
}

SegmentAnalyticsCURL::BatchEventTransfer::BatchEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse, const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents)
	: AbstractEventTransfer(httpRequest, std::move(futureResponse))
	, m_analyticEvents(analyticEvents) { }

SegmentAnalyticsCURL::BatchEventTransfer::~BatchEventTransfer() { }

SegmentAnalyticsCURL::BatchEventTransfer::BatchEventTransfer(BatchEventTransfer && eventTransfer) noexcept
	: AbstractEventTransfer(std::move(eventTransfer))
	, m_analyticEvents(std::move(eventTransfer.m_analyticEvents)) { }

const SegmentAnalyticsCURL::BatchEventTransfer & SegmentAnalyticsCURL::BatchEventTransfer::operator = (BatchEventTransfer && eventTransfer) noexcept {
	if(this != &eventTransfer) {
		AbstractEventTransfer::operator = (std::move(eventTransfer));

		m_analyticEvents = std::move(eventTransfer.m_analyticEvents);
	}

	return *this;
}

const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchEventTransfer::getAnalyticEvents() const {
	return m_analyticEvents;
}

std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchEventTransfer::getAnalyticEvents() {
	return m_analyticEvents;
}
