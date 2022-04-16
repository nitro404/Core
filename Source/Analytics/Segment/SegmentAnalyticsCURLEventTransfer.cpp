#include "SegmentAnalyticsCURL.h"

#include <Network/HTTPRequest.h>

SegmentAnalyticsCURL::AbstractEventTransfer::AbstractEventTransfer(std::shared_ptr<HTTPRequest> httpRequest, std::future<std::shared_ptr<HTTPResponse>> futureResponse)
	: m_httpRequest(httpRequest)
	, m_futureResponse(std::move(futureResponse)) { }

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

const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchEventTransfer::getAnalyticEvents() const {
	return m_analyticEvents;
}

std::vector<std::shared_ptr<SegmentAnalyticEvent>> & SegmentAnalyticsCURL::BatchEventTransfer::getAnalyticEvents() {
	return m_analyticEvents;
}
