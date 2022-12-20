#include "ZipUtilities.h"

#include <spdlog/spdlog.h>

bool ZipUtilities::isSuccess(int zipErrorCode, const std::string & errorMessage) {
	if(zipErrorCode != 0) {
		if(!errorMessage.empty()) {
			zip_error_t zipError;
			zip_error_init_with_code(&zipError, zipErrorCode);

			spdlog::error("{}: {}", errorMessage, zip_error_strerror(&zipError));

			zip_error_fini(&zipError);
		}

		return false;
	}

	return true;
}

ZipUtilities::ZipErrorHandle ZipUtilities::createZipErrorHandle() {
	zip_error_t * zipErrorHandle = new zip_error_t();
	zip_error_init(zipErrorHandle);

	return ZipErrorHandle(zipErrorHandle, [](zip_error_t * zipErrorHandle) {
		if(zipErrorHandle != nullptr) {
			zip_error_fini(zipErrorHandle);
			delete zipErrorHandle;
		}
	});
}

bool ZipUtilities::getZipArchiveInfo(zip * zipArchive, zip_stat_t * zipArchiveInfo) {
	struct zip_internal {
		zip_source_t * src;
	};

	if(zipArchive == nullptr || zipArchiveInfo == nullptr) {
		return false;
	}

	zip_stat_init(zipArchiveInfo);

	if(!ZipUtilities::isSuccess(zip_source_stat(reinterpret_cast<zip_internal *>(zipArchive)->src, zipArchiveInfo), "Failed to get zip archive information.")) {
		return false;
	}

	return true;
}
