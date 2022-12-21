#include "ZLibUtilities.h"

#include <spdlog/spdlog.h>

namespace ZLib {

	std::string resultToString(int result) {
		switch(result) {
			case Z_OK:
				return "Ok";

			case Z_STREAM_END:
				return "Stream End";

			case Z_NEED_DICT:
				return "Need Dictionary";

			case Z_ERRNO:
				return "General Error";

			case Z_STREAM_ERROR:
				return "Stream Error";

			case Z_DATA_ERROR:
				return "Data Error";

			case Z_MEM_ERROR:
				return "Memory Error";

			case Z_BUF_ERROR:
				return "Buffer Error";

			case Z_VERSION_ERROR:
				return "Version Error";
		}

		return {};
	}

	bool isSuccess(int result, const std::string & errorMessage) {
		if(result < 0) {
			if(!errorMessage.empty()) {
				spdlog::error("{}: {}.", errorMessage, result == Z_ERRNO ? std::strerror(errno) : resultToString(result));
			}

			return false;
		}

		return true;
	}

	StreamHandle createDeflationStreamHandle() {
		z_stream * streamHandle = new z_stream();
		streamHandle->zalloc = nullptr;
		streamHandle->zfree = nullptr;
		streamHandle->opaque = nullptr;

		// TODO: Allow ZLib compression parameters to be customizable:
		if(!isSuccess(deflateInit(streamHandle, Z_DEFAULT_COMPRESSION), "Failed to initialize ZLib deflation stream handle")) {
			delete streamHandle;
			return nullptr;
		}

		return StreamHandle(streamHandle, [](z_stream * streamHandle) {
			if(streamHandle != nullptr) {
				deflateEnd(streamHandle);
				delete streamHandle;
			}
		});
	}

	StreamHandle createInflationStreamHandle() {
		z_stream * streamHandle = new z_stream();
		streamHandle->zalloc = nullptr;
		streamHandle->zfree = nullptr;
		streamHandle->opaque = nullptr;

		// TODO: Allow ZLib decompression parameters to be customizable:
		if(!isSuccess(inflateInit2(streamHandle, 32), "Failed to initialize ZLib inflation stream handle")) {
			delete streamHandle;
			return nullptr;
		}

		return StreamHandle(streamHandle, [](z_stream * streamHandle) {
			if(streamHandle != nullptr) {
				inflateEnd(streamHandle);
				delete streamHandle;
			}
		});
	}

} // namespace ZLib
