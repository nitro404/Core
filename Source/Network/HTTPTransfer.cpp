#include "HTTPTransfer.h"

#include "HTTPService.h"
#include "Utilities/RapidJSONUtilities.h"
#include "Utilities/StringUtilities.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <tinyxml2.h>

static uint64_t s_transferIDCounter = 1;

HTTPTransfer::HTTPTransfer(HTTPService * service)
	: HTTPHeaders()
	, m_id(s_transferIDCounter++)
	, m_body(std::make_unique<ByteBuffer>())
	, m_service(service) { }

HTTPTransfer::HTTPTransfer(HTTPTransfer && transfer) noexcept
	: HTTPHeaders(transfer)
	, m_id(transfer.m_id)
	, m_body(std::move(transfer.m_body))
	, m_service(transfer.m_service) { }

HTTPTransfer::HTTPTransfer(const HTTPTransfer & transfer)
	: HTTPHeaders(transfer)
	, m_id(s_transferIDCounter++)
	, m_body(std::make_unique<ByteBuffer>(*transfer.m_body))
	, m_service(transfer.m_service) { }

HTTPTransfer & HTTPTransfer::operator = (HTTPTransfer && transfer) noexcept {
	if(this != &transfer) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		std::lock_guard<std::recursive_mutex> otherLock(transfer.m_mutex);

		HTTPHeaders::operator = (transfer);

		m_id = transfer.m_id;
		m_body = std::move(transfer.m_body);
		m_service = transfer.m_service;
	}

	return *this;
}

HTTPTransfer & HTTPTransfer::operator = (const HTTPTransfer & transfer) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	std::lock_guard<std::recursive_mutex> otherLock(transfer.m_mutex);

	HTTPHeaders::operator = (transfer);

	m_body = std::make_unique<ByteBuffer>(*transfer.m_body);
	m_service = transfer.m_service;

	return *this;
}

HTTPTransfer::~HTTPTransfer() { }

uint64_t HTTPTransfer::getID() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_id;
}

const ByteBuffer * HTTPTransfer::getBody() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body.get();
}

ByteBuffer * HTTPTransfer::getBody() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body.get();
}

std::string HTTPTransfer::getBodyMD5(ByteBuffer::HashFormat hashFormat) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body->getMD5(hashFormat);
}

std::string HTTPTransfer::getBodySHA1(ByteBuffer::HashFormat hashFormat) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body->getSHA1(hashFormat);
}

std::string HTTPTransfer::getBodyHash(ByteBuffer::HashType hashType, ByteBuffer::HashFormat hashFormat) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body->getHash(hashType, hashFormat);
}

std::unique_ptr<ByteBuffer> HTTPTransfer::transferBody() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::unique_ptr<ByteBuffer> body(std::move(m_body));

	m_body = std::make_unique<ByteBuffer>();

	return body;
}

std::string HTTPTransfer::getBodyAsString() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_body->toString();
}

std::unique_ptr<rapidjson::Document> HTTPTransfer::getBodyAsJSON() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::unique_ptr<rapidjson::Document> jsonDocument(std::make_unique<rapidjson::Document>());

	if(jsonDocument->Parse(reinterpret_cast<const char *>(m_body->getRawData()), m_body->getSize()).HasParseError()) {
		return nullptr;
	}

	return jsonDocument;
}

std::unique_ptr<tinyxml2::XMLDocument> HTTPTransfer::getBodyAsXML() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::unique_ptr<tinyxml2::XMLDocument> xmlDocument(std::make_unique<tinyxml2::XMLDocument>());

	if(xmlDocument->Parse(reinterpret_cast<const char *>(m_body->getRawData()), m_body->getSize()) != tinyxml2::XML_SUCCESS) {
		return nullptr;
	}

	return xmlDocument;
}

bool HTTPTransfer::setBody(const uint8_t * data, size_t size) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_body->setData(data, size);

	return true;
}

bool HTTPTransfer::setBody(const std::vector<uint8_t> & data) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_body->setData(data);

	return true;
}

bool HTTPTransfer::setBody(const std::string & value) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_body->setData(value);

	return true;
}

bool HTTPTransfer::setBody(const ByteBuffer & data) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_body->setData(data);

	return true;
}

bool HTTPTransfer::setBody(const rapidjson::Document & jsonDocument, bool updateContentType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	m_body->setData(Utilities::valueToString(jsonDocument, false));

	if(updateContentType) {
		setContentType(APPLICATION_JSON_CONTENT_TYPE);
	}

	return true;
}

bool HTTPTransfer::setBody(const tinyxml2::XMLDocument & xmlDocument, bool updateContentType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_readOnly) {
		return false;
	}

	tinyxml2::XMLPrinter printer;
	xmlDocument.Print(&printer);
	m_body->setData(printer.CStr());

	if(updateContentType) {
		setContentType(APPLICATION_XML_CONTENT_TYPE);
	}

	return true;
}

HTTPService * HTTPTransfer::getService() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_service;
}

void HTTPTransfer::clearService() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_service = nullptr;
}

bool HTTPTransfer::operator == (const HTTPTransfer & transfer) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_id == transfer.m_id;
}

bool HTTPTransfer::operator != (const HTTPTransfer & transfer) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return !operator == (transfer);
}
