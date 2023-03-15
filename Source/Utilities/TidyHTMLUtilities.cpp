#include "TidyHTMLUtilities.h"

#include <spdlog/spdlog.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>

#include <errno.h>

#include <functional>
#include <memory>

using TidyDocumentHandle = std::unique_ptr<const void, std::function<void (const void *)>>;
using TidyBufferHandle = std::unique_ptr<TidyBuffer, std::function<void (TidyBuffer *)>>;

TidyDocumentHandle createTidyDocumentHandle() {
	return TidyDocumentHandle(reinterpret_cast<const void *>(tidyCreate()), [](const void * tidyDocumentHandle) {
		tidyRelease(reinterpret_cast<TidyDoc>(tidyDocumentHandle));
	});
}

TidyBufferHandle createTidyBufferHandle() {
	TidyBuffer * tidyBuffer = new TidyBuffer();
	std::memset(tidyBuffer, 0, sizeof(TidyBuffer));

	return TidyBufferHandle(tidyBuffer, [](TidyBuffer * tidyBufferHandle) {
		tidyBufFree(tidyBufferHandle);
		delete tidyBufferHandle;
	});
}

std::string Utilities::tidyHTML(const std::string & html) {
	// documentation: https://api.html-tidy.org/tidy/tidylib_api_5.8.0

	TidyDocumentHandle documentHandle(createTidyDocumentHandle());
	TidyDoc document = reinterpret_cast<TidyDoc>(documentHandle.get());
	TidyBufferHandle outputBuffer(createTidyBufferHandle());
	TidyBufferHandle errorBuffer(createTidyBufferHandle());

	if(!tidyOptSetBool(document, TidyXhtmlOut, yes)) {
		spdlog::error("Failed to enable 'TidyXhtmlOut' option on document.");
		return {};
	}

	if(!tidyOptSetBool(document, TidyMakeBare, yes)) {
		spdlog::warn("Failed to enable 'TidyMakeBare' option on document.");
	}

	if(!tidyOptSetBool(document, TidyStyleTags, yes)) {
		spdlog::warn("Failed to enable 'TidyStyleTags' option on document.");
	}

	int result = tidySetErrorBuffer(document, errorBuffer.get());

	if(result >= 0) {
		result = tidyParseString(document, html.c_str());
	}

	if(result >= 0) {
		result = tidyCleanAndRepair(document);
	}

	if(result >= 0) {
		result = tidyRunDiagnostics(document);
	}

	if(result > 1) {
		result = (tidyOptSetBool(document, TidyForceOutput, yes) ? result : -1);
	}

	if(result >= 0) {
		result = tidySaveBuffer(document, outputBuffer.get());
	}

	if(result < 0) {
		spdlog::error("Failed to tidy HTML document with error: '{}'.", strerror(result));
		return {};
	}

	if(result > 0) {
		spdlog::debug("Tidy HTML diagnostics: {}", reinterpret_cast<const char *>(errorBuffer->bp));
	}

	return std::string(reinterpret_cast<const char *>(outputBuffer->bp), outputBuffer->size);
}
