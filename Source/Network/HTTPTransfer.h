#ifndef _HTTP_TRANSFER_H_
#define _HTTP_TRANSFER_H_

#include "ByteBuffer.h"
#include "HTTPHeaders.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <string>
#include <vector>

class HTTPService;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class HTTPTransfer : public HTTPHeaders {
public:
	HTTPTransfer(HTTPService * service);
	HTTPTransfer(HTTPTransfer && transfer) noexcept;
	HTTPTransfer(const HTTPTransfer & transfer);
	HTTPTransfer & operator = (HTTPTransfer && transfer) noexcept;
	HTTPTransfer & operator = (const HTTPTransfer & transfer);
	virtual ~HTTPTransfer();

	uint64_t getID() const;
	const ByteBuffer * getBody() const;
	ByteBuffer * getBody();
	std::string getBodyMD5(ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT) const;
	std::string getBodySHA1(ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT) const;
	std::string getBodySHA256(ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT) const;
	std::string getBodySHA512(ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT) const;
	std::string getBodyHash(ByteBuffer::HashType hashType, ByteBuffer::HashFormat hashFormat = ByteBuffer::DEFAULT_HASH_FORMAT) const;
	std::unique_ptr<ByteBuffer> transferBody();
	std::string getBodyAsString() const;
	std::unique_ptr<rapidjson::Document> getBodyAsJSON() const;
	std::unique_ptr<tinyxml2::XMLDocument> getBodyAsXML() const;
	bool setBody(const uint8_t * data, size_t size);
	bool setBody(const std::vector<uint8_t> & data);
	bool setBody(const std::string & value);
	bool setBody(const ByteBuffer & data);
	bool setBody(const rapidjson::Document & jsonDocument, bool updateContentType = true);
	bool setBody(const tinyxml2::XMLDocument & xmlDocument, bool updateContentType = true);

	bool operator == (const HTTPTransfer & transfer) const;
	bool operator != (const HTTPTransfer & transfer) const;

protected:
	HTTPService * getService() const;
	void clearService();

	uint64_t m_id;
	std::unique_ptr<ByteBuffer> m_body;
	HTTPService * m_service;
};

#endif // _HTTP_TRANSFER_H_
