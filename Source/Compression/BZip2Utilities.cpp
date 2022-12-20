#include "BZip2Utilities.h"

#include <spdlog/spdlog.h>

namespace BZip2 {

	std::string resultToString(int result) {
		switch(result) {
			case BZ_OK:
				return "Ok";

			case BZ_RUN_OK:
				return "Run Ok";

			case BZ_FLUSH_OK:
				return "Flush Ok";

			case BZ_FINISH_OK:
				return "Finish Ok";

			case BZ_STREAM_END:
				return "Stream End";

			case BZ_SEQUENCE_ERROR:
				return "Sequence Error";

			case BZ_PARAM_ERROR:
				return "Parameter Error";

			case BZ_MEM_ERROR:
				return "Memory Error";

			case BZ_DATA_ERROR:
				return "Data Error";

			case BZ_DATA_ERROR_MAGIC:
				return "Data Error Magic";

			case BZ_IO_ERROR:
				return "I/O Error";

			case BZ_UNEXPECTED_EOF:
				return "Unexpected End of File";

			case BZ_OUTBUFF_FULL:
				return "Output Buffer Full";

			case BZ_CONFIG_ERROR:
				return "Configuration Error";
		}

		return {};
	}

	bool isSuccess(int result, const std::string & errorMessage) {
		if(result < 0) {
			if(!errorMessage.empty()) {
				spdlog::error("{}: {}.", errorMessage, resultToString(result));
			}

			return false;
		}

		return true;
	}

	StreamHandle createCompressionStreamHandle() {
		bz_stream * streamHandle = new bz_stream();
		streamHandle->bzalloc = nullptr;
		streamHandle->bzfree = nullptr;
		streamHandle->opaque = nullptr;

		// TODO: Allow BZip2 compression initialization parameters to be configurable
		if(!isSuccess(BZ2_bzCompressInit(streamHandle, 9, 0, 30), "Failed to initialize BZip2 compression stream handle")) {
			delete streamHandle;
			return nullptr;
		}

		return StreamHandle(streamHandle, [](bz_stream * streamHandle) {
			if(streamHandle != nullptr) {
				BZ2_bzCompressEnd(streamHandle);
				delete streamHandle;
			}
		});
	}

	StreamHandle createDecompressionStreamHandle() {
		bz_stream * streamHandle = new bz_stream();
		streamHandle->bzalloc = nullptr;
		streamHandle->bzfree = nullptr;
		streamHandle->opaque = nullptr;

		// TODO: Allow BZip2 decompression initialization parameters to be configurable
		if(!isSuccess(BZ2_bzDecompressInit(streamHandle, 0, 0), "Failed to initialize BZip2 decompression stream handle")) {
			delete streamHandle;
			return nullptr;
		}

		return StreamHandle(streamHandle, [](bz_stream * streamHandle) {
			if(streamHandle != nullptr) {
				BZ2_bzDecompressEnd(streamHandle);
				delete streamHandle;
			}
		});
	}

} // namespace BZip2
