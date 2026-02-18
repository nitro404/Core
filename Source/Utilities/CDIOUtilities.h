#ifndef _CDIO_UTILITIES_H_
#define _CDIO_UTILITIES_H_

#include "ByteBuffer.h"
#include <cdio/cdio.h>
#include <cdio/logging.h>
#include <cdio++/cdio.hpp>
#include <cdio++/iso9660.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace CDIOUtilities {

	extern const std::string rootDirectoryPath;

	driver_id_t getDriverIDFromFileExtension(const std::string_view filePathOrExtension);
	std::string driverIDToString(driver_id_t driverID);
	spdlog::level::level_enum cdioLogLevelToSpdlogLogLevel(cdio_log_level_t logLevel);
	cdio_log_level_t spdlogLogLevelToCDIOLogLevel(spdlog::level::level_enum logLevel);
	std::string cdioLogLevelToString(cdio_log_level_t logLevel);
	std::vector<std::unique_ptr<ISO9660::Stat>> makeStatistics(const stat_vector_t & statVector, bool omitNagivationEntries = true);
	std::unique_ptr<ISO9660::FS> readDiscImage(const std::string & filePath);
	std::vector<std::unique_ptr<ISO9660::Stat>> getDirectoryContentsStatistics(ISO9660::FS & isoFileSystem, const std::string & directoryPath, bool * error);
	std::optional<std::vector<std::unique_ptr<ISO9660::Stat>>> getDirectoryContentsStatistics(ISO9660::FS & isoFileSystem, const std::string & directoryPath);
	std::vector<std::unique_ptr<ISO9660::Stat>> getRootDirectoryContentsStatistics(ISO9660::FS & isoFileSystem, bool * error);
	std::optional<std::vector<std::unique_ptr<ISO9660::Stat>>> getRootDirectoryContentsStatistics(ISO9660::FS & isoFileSystem);
	std::string getApplication(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPreparer(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPublisher(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getSystem(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getVolume(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getVolumeSet(ISO9660::PVD & primaryVolumeDescriptor);
	bool isFile(const iso9660_stat_t & statistic);
	bool isFile(const ISO9660::Stat & statistic);
	bool isFile(ISO9660::FS & isoFileSystem, const std::string & filePath);
	bool isDirectory(const iso9660_stat_t & statistic);
	bool isDirectory(const ISO9660::Stat & statistic);
	bool isDirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath);
	std::string translateName(const std::string & originalName);
	std::string getFileName(const iso9660_stat_t & statistic);
	std::string getFileName(const ISO9660::Stat & statistic);
	uint32_t getFileSize(const iso9660_stat_t & statistic);
	uint32_t getFileSize(const ISO9660::Stat & statistic);
	uint32_t getFileSize(ISO9660::FS & isoFileSystem, const std::string & filePath);
	lsn_t getFileStartLogicalSectorNumber(const iso9660_stat_t & statistic);
	lsn_t getFileStartLogicalSectorNumber(const ISO9660::Stat & statistic);
	lsn_t getFileStartLogicalSectorNumber(ISO9660::FS & isoFileSystem, const std::string & filePath);
	bool hasFile(ISO9660::FS & isoFileSystem, const std::string & filePath);
	bool hasDirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath);
	std::vector<std::string> getDirectoryContents(ISO9660::FS & isoFileSystem, const std::string & directoryPath, bool * error);
	std::optional<std::vector<std::string>> getDirectoryContents(ISO9660::FS & isoFileSystem, const std::string & directoryPath);
	std::vector<std::string> getRootDirectoryContents(ISO9660::FS & isoFileSystem, bool * error);
	std::optional<std::vector<std::string>> getRootDirectoryContents(ISO9660::FS & isoFileSystem);
	std::unique_ptr<ByteBuffer> readFile(ISO9660::FS & isoFileSystem, lsn_t logicalSectorNumber, long int fileSize);
	std::unique_ptr<ByteBuffer> readFile(ISO9660::FS & isoFileSystem, ISO9660::Stat & statistic);
	std::unique_ptr<ByteBuffer> readFile(ISO9660::FS & isoFileSystem, const std::string & filePath);

}

#endif // _CDIO_UTILITIES_H_
