#ifndef _ZIP_UTILITIES_H_
#define _ZIP_UTILITIES_H_

#include <zip.h>

#include <functional>
#include <string>

namespace ZipUtilities {

	using ZipErrorHandle = std::unique_ptr<zip_error_t, std::function<void (zip_error_t *)>>;

	bool isSuccess(int zipError, const std::string & errorMessage = {});
	ZipErrorHandle createZipErrorHandle();
	bool getZipArchiveInfo(zip * zipArchive, zip_stat_t * zipArchiveInfo);

}

#endif // _ZIP_UTILITIES_H_
