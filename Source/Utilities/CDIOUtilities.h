#ifndef _CDIO_UTILITIES_H_
#define _CDIO_UTILITIES_H_

#include <cdio/cdio.h>
#include <cdio/logging.h>
#include <cdio++/cdio.hpp>
#include <cdio++/iso9660.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace CDIOUtilities {

	driver_id_t getDriverIDFromFileExtension(const std::string_view filePathOrExtension);
	std::string driverIDToString(driver_id_t driverID);
	spdlog::level::level_enum cdioLogLevelToSpdlogLogLevel(cdio_log_level_t logLevel);
	cdio_log_level_t spdlogLogLevelToCDIOLogLevel(spdlog::level::level_enum logLevel);
	std::string cdioLogLevelToString(cdio_log_level_t logLevel);
	std::vector<std::unique_ptr<ISO9660::Stat>> makeISOStatistics(const stat_vector_t & statVector);
	std::vector<std::unique_ptr<ISO9660::Stat>> readISODirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath, bool * error);
	std::optional<std::vector<std::unique_ptr<ISO9660::Stat>>> readISODirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath);
	std::string getPVDApplicationID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDPreparerID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDPublisherID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDSystemID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDVolumeID(ISO9660::PVD & primaryVolumeDescriptor);
	std::string getPVDVolumeSetID(ISO9660::PVD & primaryVolumeDescriptor);
	bool isFile(const iso9660_stat_t & statistic);
	bool isFile(const ISO9660::Stat & statistic);
	bool isDirectory(const iso9660_stat_t & statistic);
	bool isDirectory(const ISO9660::Stat & statistic);
	std::string translateISOName(const std::string & originalName);
	std::string getFileName(const iso9660_stat_t & statistic);
	std::string getFileName(const ISO9660::Stat & statistic);
	uint32_t getFileSize(const iso9660_stat_t & statistic);
	uint32_t getFileSize(const ISO9660::Stat & statistic);
	lsn_t getFileStartLogicalSectorNumber(const iso9660_stat_t & statistic);
	lsn_t getFileStartLogicalSectorNumber(const ISO9660::Stat & statistic);

}

#endif // _CDIO_UTILITIES_H_
