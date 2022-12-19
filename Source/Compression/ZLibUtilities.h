#ifndef _ZLIB_UTILITIES_H_
#define _ZLIB_UTILITIES_H_

#include <zlib.h>

#include <functional>
#include <memory>
#include <string>

namespace ZLib {

	using StreamHandle = std::unique_ptr<z_stream, std::function<void (z_stream *)>>;

	std::string resultToString(int result);
	bool isSuccess(int result, const std::string & errorMessage = {});
	StreamHandle createDeflationStreamHandle();
	StreamHandle createInflationStreamHandle();

}

#endif // _ZLIB_UTILITIES_H_
