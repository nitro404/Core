#ifndef _BZIP2_UTILITIES_H_
#define _BZIP2_UTILITIES_H_

#include <bzlib.h>

#include <functional>
#include <memory>
#include <string>

namespace BZip2 {

	using StreamHandle = std::unique_ptr<bz_stream, std::function<void (bz_stream *)>>;

	std::string resultToString(int result);
	bool isSuccess(int result, const std::string & errorMessage = {});
	StreamHandle createCompressionStreamHandle();
	StreamHandle createDecompressionStreamHandle();

}

#endif // _BZIP2_UTILITIES_H_
