#include "SegmentAnalyticsCURL.h"

#include "Network/HTTPService.h"
#include "SegmentAnalyticEvent.h"
#include "Utilities/FileUtilities.h"
#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <chrono>
#include <limits>

using namespace std::chrono_literals;

const std::string SegmentAnalyticsCURL::DEFAULT_API_ADDRESS = "https://api.segment.io/v1";

SegmentAnalyticsCURL::SegmentAnalyticsCURL()
	: SegmentAnalytics()
	, m_running(false)
	, m_flushRequested(false)
	, m_stopRequested(false)
	, m_apiAddress(DEFAULT_API_ADDRESS) { }

SegmentAnalyticsCURL::~SegmentAnalyticsCURL() {
	stop();
}

bool SegmentAnalyticsCURL::isRunning() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_running;
}

bool SegmentAnalyticsCURL::initialize(const Configuration & configuration) {
	if(!SegmentAnalytics::initialize(configuration)) {
		return false;
	}

	m_failedNetworkTransferRetryDelay = configuration.failedNetworkTransferRetryDelay;

	DataStorage * dataStorage = getDataStorage();
	std::vector<std::shared_ptr<SegmentAnalyticEvent>> cachedAnalyticEvents(dataStorage->getPendingAnalyticEvents());

	for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = cachedAnalyticEvents.cbegin(); i != cachedAnalyticEvents.cend(); ++i) {
		m_queuedEvents.push_back(*i);
	}

	return true;
}

bool SegmentAnalyticsCURL::shouldTrackApplicationEvents() const {
	return true;
}

bool SegmentAnalyticsCURL::flush(std::chrono::milliseconds waitForDuration) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized() || !m_running) {
		return false;
	}

	m_flushRequested = true;

	lock.unlock();

	m_waitCondition.notify_one();

	if(waitForDuration != 0ms) {
		lock.lock();

		if(!m_queuedEvents.empty() || !m_analyticEventTransfers.empty()) {
			lock.unlock();

			std::chrono::time_point<std::chrono::system_clock> flushStartTimePoint = std::chrono::system_clock::now();

			std::unique_lock<std::recursive_mutex> flushLock(m_flushMutex);
			m_flushWaitCondition.wait_for(flushLock, waitForDuration);

			std::chrono::milliseconds flushWaitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - flushStartTimePoint);

#if _DEBUG
			fmt::print("Waited for {} ms to flush pending analytic events.\n", flushWaitDuration.count());
#endif
		}
	}

	return true;
}

bool SegmentAnalyticsCURL::queueEvent(std::unique_ptr<SegmentAnalyticEvent> analyticEvent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized()) {
		return false;
	}

	std::shared_ptr<SegmentAnalyticEvent> sharedAnalyticEvent(std::shared_ptr<SegmentAnalyticEvent>(analyticEvent.release()));

	DataStorage * dataStorage = getDataStorage();

	if(!dataStorage->addPendingAnalyticEvent(sharedAnalyticEvent)) {
		return false;
	}

	m_queuedEvents.push_back(sharedAnalyticEvent);

	m_waitCondition.notify_one();

	return true;
}

bool SegmentAnalyticsCURL::sendSingleAnalyticEvent(std::shared_ptr<SegmentAnalyticEvent> analyticEvent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized() || !SegmentAnalyticEvent::isValid(analyticEvent.get())) {
		return false;
	}

	std::shared_ptr<HTTPRequest> singleRequest(createSingleAnalyticEventRequest(*analyticEvent));

	if(singleRequest == nullptr) {
		return false;
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(getHTTPService()->sendRequest(singleRequest));

	if(!futureResponse.valid()) {
		return false;
	}

	m_analyticEventTransfers[singleRequest->getID()] = std::unique_ptr<SingleEventTransfer>(new SingleEventTransfer(singleRequest, std::move(futureResponse), analyticEvent));

	return true;
}

bool SegmentAnalyticsCURL::sendAnalyticEventBatch(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized()) {
		return false;
	}

	std::shared_ptr<HTTPRequest> batchRequest(createBatchAnalyticEventRequest(analyticEvents));

	if(batchRequest == nullptr) {
		return false;
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(getHTTPService()->sendRequest(batchRequest));

	if(!futureResponse.valid()) {
		return false;
	}

	m_analyticEventTransfers[batchRequest->getID()] = std::unique_ptr<BatchEventTransfer>(new BatchEventTransfer(batchRequest, std::move(futureResponse), analyticEvents));

	return true;
}

const SegmentAnalytics::LibraryInfoProvider * SegmentAnalyticsCURL::getLibraryInfoProvider() const {
	return this;
}

const std::string & SegmentAnalyticsCURL::getLibraryName() const {
	static const std::string LIBRARY_NAME("segment-curl");
	return LIBRARY_NAME;
}

const std::string & SegmentAnalyticsCURL::getLibraryVersion() const {
	static const std::string LIBRARY_VERSION("1.0.0");
	return LIBRARY_VERSION;
}

std::shared_ptr<HTTPRequest> SegmentAnalyticsCURL::createSingleAnalyticEventRequest(const SegmentAnalyticEvent & analyticEvent) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized()) {
		return nullptr;
	}

	std::string apiEndpoint(Utilities::toLowerCase(magic_enum::enum_name(analyticEvent.getType())));

	std::shared_ptr<HTTPRequest> singleRequest(getHTTPService()->createRequest(HTTPRequest::Method::Post, Utilities::joinPaths(m_apiAddress, apiEndpoint)));

	configureAnalyticEventRequest(*singleRequest);

	std::unique_ptr<rapidjson::Document> bodyDocument(createBaseEventPayloadDocument());

	if(!addEventDataToValue(analyticEvent, getAnonymousID(), *bodyDocument, bodyDocument->GetAllocator(), false)) {
		fmt::print("Failed to convert analytic event values to JSON data.\n");
		return nullptr;
	}

	singleRequest->setBody(*bodyDocument);

	return singleRequest;
}

std::shared_ptr<HTTPRequest> SegmentAnalyticsCURL::createBatchAnalyticEventRequest(const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized()) {
		return nullptr;
	}

	std::shared_ptr<HTTPRequest> batchRequest(getHTTPService()->createRequest(HTTPRequest::Method::Post, Utilities::joinPaths(m_apiAddress, "batch")));

	configureAnalyticEventRequest(*batchRequest);

	std::unique_ptr<rapidjson::Document> bodyDocument(createBaseEventPayloadDocument());
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = bodyDocument->GetAllocator();

	rapidjson::Value batchEventArrayValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = analyticEvents.cbegin(); i != analyticEvents.cend(); ++i) {
		rapidjson::Value eventArrayEntryValue(rapidjson::kObjectType);

		if(!addEventDataToValue(**i, getAnonymousID(), eventArrayEntryValue, allocator, true)) {
			fmt::print("Failed to convert analytic event values to JSON data.\n");
			return nullptr;
		}

		batchEventArrayValue.PushBack(eventArrayEntryValue, allocator);
	}

	bodyDocument->AddMember(rapidjson::StringRef("batch"), batchEventArrayValue, allocator);

	batchRequest->setBody(*bodyDocument);

	return batchRequest;
}

void SegmentAnalyticsCURL::configureAnalyticEventRequest(HTTPRequest & request) {
	request.setAuthorization(getWriteKey(), Utilities::emptyString);
	request.setAcceptedEncodingTypes(HTTPRequest::EncodingTypes::Deflate | HTTPRequest::EncodingTypes::GZip);
}

bool SegmentAnalyticsCURL::start() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!isInitialized() || isStarted() || m_running || !SegmentAnalytics::start()) {
		return false;
	}

	m_running = true;

	m_analyticEventThread = AnalyticEventThread(new std::thread(&SegmentAnalyticsCURL::run, this), [](std::thread * analyticEventThread) {
		if(analyticEventThread != nullptr) {
			if(analyticEventThread->joinable()) {
				analyticEventThread->join();
			}

			delete analyticEventThread;
		}
	});

	return true;
}

void SegmentAnalyticsCURL::stop() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	SegmentAnalytics::stop();

	m_stopRequested = true;

	lock.unlock();

	m_waitCondition.notify_one();

	m_analyticEventThread.reset();

	lock.lock();

	m_running = false;
}

void SegmentAnalyticsCURL::run() {
	if(!isInitialized() || !m_running) {
		return;
	}

	DataStorage * dataStorage = getDataStorage();
	std::shared_ptr<HTTPService> httpService(getHTTPService());
	bool batchMode = isUsingBatchMode();
	uint16_t maxEventQueueSize = getMaxEventQueueSize();
	std::vector<std::shared_ptr<SegmentAnalyticEvent>> analyticEventsToSend;
	std::vector<std::shared_ptr<SegmentAnalyticEvent>> analyticEventsToRemove;
	std::vector<uint64_t> analyticEventTransferRequestIdentifiersToErase;
	std::vector<const AbstractFailedEvent *> resentFailedEventsToErase;

	while(true) {
		std::unique_lock<std::recursive_mutex> lock(m_mutex);

		// clear all pending analytic events and abort all in-progress analytic event transfers, then terminate thread execution
		if(m_stopRequested) {
			m_queuedEvents.clear();

			for(std::map<uint64_t, std::unique_ptr<AbstractEventTransfer>>::const_iterator i = m_analyticEventTransfers.cbegin(); i != m_analyticEventTransfers.cend(); ++i) {
				httpService->abortRequest(i->second->getRequest());
			}

			m_analyticEventTransfers.clear();

			return;
		}

		// get all pending analytic events ready to transfer if a flush was requested or the service is not running in batch mode
		if(!m_queuedEvents.empty()) {
			if(!batchMode || m_flushRequested) {
				std::shared_ptr<HTTPRequest> analyticEvent;

				while(!m_queuedEvents.empty()) {
					analyticEventsToSend.emplace_back(m_queuedEvents.front());
					m_queuedEvents.pop_front();
				}
			}
			else if(batchMode && m_queuedEvents.size() >= maxEventQueueSize) {
				for(size_t i = 0; i < maxEventQueueSize; i++) {
					analyticEventsToSend.emplace_back(m_queuedEvents.front());
					m_queuedEvents.pop_front();
				}
			}
		}

		// retry any failed analytic event network transfers if the retry delay has expired
		if(!m_failedEvents.empty()) {
			for(std::vector<std::unique_ptr<AbstractFailedEvent>>::const_iterator i = m_failedEvents.cbegin(); i != m_failedEvents.cend(); ++i) {
				if(!(*i)->shouldRetryTransfer() && !m_flushRequested) {
					continue;
				}

				fmt::print("Re-trying failed Segment analytics event network transfer.\n");

				const SingleFailedEvent * singleFailedEvent = dynamic_cast<const SingleFailedEvent *>(i->get());
				const BatchFailedEvents * batchFailedEvents = dynamic_cast<const BatchFailedEvents *>(i->get());

				if(singleFailedEvent != nullptr) {
					analyticEventsToSend.emplace_back(singleFailedEvent->getAnalyticEvent());
				}
				else if(batchFailedEvents != nullptr) {
					const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents = batchFailedEvents->getAnalyticEvents();

					for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator j = analyticEvents.cbegin(); j != analyticEvents.cend(); ++j) {
						analyticEventsToSend.emplace_back(*j);
					}
				}

				resentFailedEventsToErase.emplace_back(i->get());
			}

			// remove any re-sent failed analytic events from the cache
			if(!resentFailedEventsToErase.empty()) {
				for(std::vector<const AbstractFailedEvent *>::const_iterator i = resentFailedEventsToErase.cbegin(); i != resentFailedEventsToErase.cend(); ++i) {
					for(std::vector<std::unique_ptr<AbstractFailedEvent>>::const_iterator j = m_failedEvents.cbegin(); j != m_failedEvents.cend(); ++j) {
						if(*i != j->get()) {
							continue;
						}

						m_failedEvents.erase(j);

						break;
					}
				}

				resentFailedEventsToErase.clear();
			}
		}

		// start network transfers for pending analytic events
		if(!analyticEventsToSend.empty()) {
			std::sort(analyticEventsToSend.begin(), analyticEventsToSend.end(), [](std::shared_ptr<SegmentAnalyticEvent> eventA, std::shared_ptr<SegmentAnalyticEvent> eventB) {
				return eventA->getID() < eventB->getID();
			});

			if(batchMode) {
				sendAnalyticEventBatch(analyticEventsToSend);
			}
			else {
				for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator i = analyticEventsToSend.cbegin(); i != analyticEventsToSend.cend(); ++i) {
					sendSingleAnalyticEvent(*i);
				}
			}

			analyticEventsToSend.clear();
		}

		// reset flush requested flag since all pending analytic events have been queued
		m_flushRequested = false;

		// check for and remove successful and failed analytic event network transfers
		if(!m_analyticEventTransfers.empty()) {
			for(std::map<uint64_t, std::unique_ptr<AbstractEventTransfer>>::const_iterator i = m_analyticEventTransfers.cbegin(); i != m_analyticEventTransfers.cend(); ++i) {
				if(i->second->getFutureResponse().wait_for(0ms) != std::future_status::ready) {
					continue;
				}

				std::shared_ptr<HTTPResponse> response(i->second->getFutureResponse().get());

				if(!response->isFailure()) {
					const SingleEventTransfer * singleEventTransfer = dynamic_cast<const SingleEventTransfer *>(i->second.get());
					const BatchEventTransfer * batchEventTransfer = dynamic_cast<const BatchEventTransfer *>(i->second.get());
					bool shouldRemoveTransferAnalyticEvents = false;
					HTTPStatusCode responseStatusCode = magic_enum::enum_cast<HTTPStatusCode>(response->getStatusCode()).value_or(HTTPStatusCode::None);

					if(response->isSuccessStatusCode()) {
						shouldRemoveTransferAnalyticEvents = true;
					}
					else {
						fmt::print("Segment analytics network transfer failed with status: {}{}.\n", response->getStatusCode(), responseStatusCode == HTTPStatusCode::None ? "" : " " + Utilities::toCapitalCase(magic_enum::enum_name(responseStatusCode)));

						if(responseStatusCode == HTTPStatusCode::BadRequest ||
						   responseStatusCode == HTTPStatusCode::PayloadTooLarge ||
						   responseStatusCode == HTTPStatusCode::UnprocessableEntity) {
							fmt::print("Cancelling and removing failed Segment analytics event network transfer.\n");

							shouldRemoveTransferAnalyticEvents = true;
						}
						else {
							fmt::print("Re-trying failed Segment analytics event network transfer in {} ms.\n", m_failedNetworkTransferRetryDelay.count());

							if(batchEventTransfer != nullptr) {
								m_failedEvents.emplace_back(std::make_unique<BatchFailedEvents>(m_failedNetworkTransferRetryDelay, batchEventTransfer->getAnalyticEvents()));
							}
							else if(singleEventTransfer != nullptr) {
								m_failedEvents.emplace_back(std::make_unique<SingleFailedEvent>(m_failedNetworkTransferRetryDelay, singleEventTransfer->getAnalyticEvent()));
							}
						}
					}

					if(shouldRemoveTransferAnalyticEvents) {
						if(batchEventTransfer != nullptr) {
							const std::vector<std::shared_ptr<SegmentAnalyticEvent>> & analyticEvents = batchEventTransfer->getAnalyticEvents();

							for(std::vector<std::shared_ptr<SegmentAnalyticEvent>>::const_iterator j = analyticEvents.cbegin(); j != analyticEvents.cend(); ++j) {
								analyticEventsToRemove.push_back(*j);
							}
						}
						else if(singleEventTransfer != nullptr) {
							analyticEventsToRemove.push_back(singleEventTransfer->getAnalyticEvent());
						}
					}
				}

				analyticEventTransferRequestIdentifiersToErase.push_back(i->second->getRequest()->getID());
			}
		}

		// remove all completed and failed analytic event transfer data containers
		if(!analyticEventTransferRequestIdentifiersToErase.empty()) {
			for(std::vector<uint64_t>::const_iterator i = analyticEventTransferRequestIdentifiersToErase.cbegin(); i != analyticEventTransferRequestIdentifiersToErase.cend(); ++i) {
				m_analyticEventTransfers.erase(*i);
			}

			analyticEventTransferRequestIdentifiersToErase.clear();
		}

		// remove successfully transmitted analytic events from cache
		if(!analyticEventsToRemove.empty()) {
			dataStorage->removePendingAnalyticEvents(analyticEventsToRemove);
			analyticEventsToRemove.clear();
		}

		// notify any threads waiting for a flush to complete
		if(m_analyticEventTransfers.empty()) {
			m_flushWaitCondition.notify_all();
		}

		// delay next processing cycle based on how many analytic events are queued and how many transfers are in progress
		if(((batchMode && m_queuedEvents.size() < maxEventQueueSize) || (!batchMode && m_queuedEvents.empty())) && m_analyticEventTransfers.empty() && m_failedEvents.empty()) {
			m_waitCondition.wait(lock);
		}
		else if(m_queuedEvents.size() >= maxEventQueueSize) {
			std::this_thread::sleep_for(10ms);
		}
		else if(!m_analyticEventTransfers.empty()) {
			std::this_thread::sleep_for(100ms);
		}
		else if(!m_failedEvents.empty()) {
			std::this_thread::sleep_for(1s);
		}
	}
}
