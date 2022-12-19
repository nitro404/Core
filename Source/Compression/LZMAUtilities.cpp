#include "LZMAUtilities.h"

#include <spdlog/spdlog.h>

namespace LZMA {

	std::string statusToString(lzma_ret status) {
		switch(status) {
			case LZMA_OK:
				return "Ok";

			case LZMA_STREAM_END:
				return "Stream End";

			case LZMA_NO_CHECK:
				return "No Check";

			case LZMA_UNSUPPORTED_CHECK:
				return "Unsupported Check";

			case LZMA_GET_CHECK:
				return "Get Check";

			case LZMA_MEM_ERROR:
				return "Memory Error";

			case LZMA_MEMLIMIT_ERROR:
				return "Memory Limit Error";

			case LZMA_FORMAT_ERROR:
				return "Format Error";

			case LZMA_OPTIONS_ERROR:
				return "Options Error";

			case LZMA_DATA_ERROR:
				return "Data Error";

			case LZMA_BUF_ERROR:
				return "Buffer Error";

			case LZMA_PROG_ERROR:
				return "Program Error";
		}

		return {};
	}

	bool isSuccess(lzma_ret status, const std::string & errorMessage) {
		if(status != LZMA_OK) {
			if(!errorMessage.empty()) {
				spdlog::error("{} {}", errorMessage, statusToString(status));
			}

			return false;
		}

		return true;
	}

	StreamHandle createStreamHandle() {
		return StreamHandle(new lzma_stream(LZMA_STREAM_INIT), [](lzma_stream * streamHandle) {
			if(streamHandle != nullptr) {
				lzma_end(streamHandle);
				delete streamHandle;
			}
		});
	}

}
