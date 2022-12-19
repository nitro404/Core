#ifndef _LZMA_UTILITIES_H_
#define _LZMA_UTILITIES_H_

#include <lzma.h>

#include <functional>
#include <memory>
#include <string>

namespace LZMA {

	using StreamHandle = std::unique_ptr<lzma_stream, std::function<void (lzma_stream *)>>;

	std::string statusToString(lzma_ret status);
	bool isSuccess(lzma_ret status, const std::string & errorMessage = {});
	StreamHandle createStreamHandle();

}

#endif // _LZMA_UTILITIES_H_
