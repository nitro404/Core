#include "CDIOUtilities.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"

static std::string convertRawString(const char * rawData) {
	if(rawData == nullptr) {
		return {};
	}

	return rawData;
}

static std::string convertAndFreeRawString(char * rawData) {
	if(rawData == nullptr) {
		return {};
	}

	std::string data(rawData);

	free(rawData);

	return data;
}


driver_id_t CDIOUtilities::getDriverIDFromFileExtension(const std::string_view filePathOrExtension) {
	if(filePathOrExtension.empty()) {
		return DRIVER_UNKNOWN;
	}

	std::string_view fileExtension(Utilities::getFileExtension(filePathOrExtension));

	if(fileExtension.empty()) {
		return DRIVER_UNKNOWN;
	}

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "cue")) {
		return DRIVER_BINCUE;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "nrg")) {
		return DRIVER_NRG;
	}

	return DRIVER_UNKNOWN;
}

std::string CDIOUtilities::driverIDToString(driver_id_t driverID) {
	switch(driverID) {
		case DRIVER_UNKNOWN:
			return "Unknown";
		case DRIVER_AIX:
			return "AIX";
		case DRIVER_FREEBSD:
			return "FreeBSD";
		case DRIVER_NETBSD:
			return "NetBSD";
		case DRIVER_LINUX:
			return "Linux";
		case DRIVER_SOLARIS:
			return "Solaris";
		case DRIVER_OSX:
			return "OSX";
		case DRIVER_WIN32:
			return "Win32";
		case DRIVER_CDRDAO:
			return "CDRDAO";
		case DRIVER_BINCUE:
			return "BinCue";
		case DRIVER_NRG:
			return "NRG";
		case DRIVER_DEVICE:
			return "Device";
	}

	return "";
}

spdlog::level::level_enum CDIOUtilities::cdioLogLevelToSpdlogLogLevel(cdio_log_level_t logLevel) {
	switch(logLevel) {
		case CDIO_LOG_DEBUG:
			return spdlog::level::level_enum::debug;
		case CDIO_LOG_INFO:
			return spdlog::level::level_enum::info;
		case CDIO_LOG_WARN:
			return spdlog::level::level_enum::warn;
		case CDIO_LOG_ERROR:
			return spdlog::level::level_enum::err;
		case CDIO_LOG_ASSERT:
			return spdlog::level::level_enum::critical;
	}

	return spdlog::level::level_enum::debug;
}

cdio_log_level_t CDIOUtilities::spdlogLogLevelToCDIOLogLevel(spdlog::level::level_enum logLevel) {
	switch(logLevel) {
		case spdlog::level::level_enum::trace:
		case spdlog::level::level_enum::debug:
			return CDIO_LOG_DEBUG;
		case spdlog::level::level_enum::info:
			return CDIO_LOG_INFO;
		case spdlog::level::level_enum::warn:
			return CDIO_LOG_WARN;
		case spdlog::level::level_enum::err:
			return CDIO_LOG_ERROR;
		case spdlog::level::level_enum::critical:
			return CDIO_LOG_ASSERT;
		case spdlog::level::level_enum::off:
		case spdlog::level::level_enum::n_levels:
			break;
	}

	return CDIO_LOG_DEBUG;
}

std::string CDIOUtilities::cdioLogLevelToString(cdio_log_level_t logLevel) {
	switch(logLevel) {
		case CDIO_LOG_DEBUG:
			return "Debug";
		case CDIO_LOG_INFO:
			return "Info";
		case CDIO_LOG_WARN:
			return "Warning";
		case CDIO_LOG_ERROR:
			return "Error";
		case CDIO_LOG_ASSERT:
			return "Assert";
	}

	return "";
}

std::vector<std::unique_ptr<ISO9660::Stat>> CDIOUtilities::makeISOStatistics(const stat_vector_t & statistics) {
	std::vector<std::unique_ptr<ISO9660::Stat>> newStatistics;

	for(stat_vector_t::const_iterator statsVectorIterator = statistics.cbegin(); statsVectorIterator != statistics.cend(); ++statsVectorIterator) {
		newStatistics.emplace_back(*statsVectorIterator);
	}

	return newStatistics;
}

std::vector<std::unique_ptr<ISO9660::Stat>> CDIOUtilities::readISODirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath, bool * error) {
	stat_vector_t originalStatistics;

	if(!isoFileSystem.readdir(directoryPath.data(), originalStatistics)) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return makeISOStatistics(originalStatistics);
}

std::optional<std::vector<std::unique_ptr<ISO9660::Stat>>> CDIOUtilities::readISODirectory(ISO9660::FS & isoFileSystem, const std::string & directoryPath) {
	bool error = false;

	std::vector<std::unique_ptr<ISO9660::Stat>> statistics(readISODirectory(isoFileSystem, directoryPath, &error));

	if(error) {
		return {};
	}

	return statistics;
}

std::string CDIOUtilities::getApplication(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_application_id());
}

std::string CDIOUtilities::getPreparer(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_preparer_id());
}

std::string CDIOUtilities::getPublisher(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_publisher_id());
}

std::string CDIOUtilities::getID(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertRawString(primaryVolumeDescriptor.get_pvd_id());
}

std::string CDIOUtilities::getSystem(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_system_id());
}

std::string CDIOUtilities::getVolume(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_volume_id());
}

std::string CDIOUtilities::getVolumeSet(ISO9660::PVD & primaryVolumeDescriptor) {
	return convertAndFreeRawString(primaryVolumeDescriptor.get_volumeset_id());
}

bool CDIOUtilities::isFile(const iso9660_stat_t & statistic) {
	return statistic.type == iso9660_stat_s::_STAT_FILE;
}

bool CDIOUtilities::isFile(const ISO9660::Stat & statistic) {
	if(statistic.p_stat == nullptr) {
		return false;
	}

	return isFile(*statistic.p_stat);
}

bool CDIOUtilities::isDirectory(const iso9660_stat_t & statistic) {
	return statistic.type == iso9660_stat_s::_STAT_DIR;
}

bool CDIOUtilities::isDirectory(const ISO9660::Stat & statistic) {
	if(statistic.p_stat == nullptr) {
		return false;
	}

	return isDirectory(*statistic.p_stat);
}

std::string CDIOUtilities::translateName(const std::string & originalName) {
	std::string newName(originalName.length() + 1, '\0');

	int newNameLength = iso9660_name_translate(originalName.data(), newName.data());
	newName.resize(newNameLength, '\0');

	return newName;
}

std::string CDIOUtilities::getFileName(const iso9660_stat_t & statistic) {
	return translateName(statistic.filename);
}

std::string CDIOUtilities::getFileName(const ISO9660::Stat & statistic) {
	if(statistic.p_stat == nullptr) {
		return {};
	}

	return getFileName(*statistic.p_stat);
}

uint32_t CDIOUtilities::getFileSize(const iso9660_stat_t & statistic) {
	return statistic.size;
}

uint32_t CDIOUtilities::getFileSize(const ISO9660::Stat & statistic) {
	if(statistic.p_stat == nullptr) {
		return 0u;
	}

	return getFileSize(*statistic.p_stat);
}

lsn_t CDIOUtilities::getFileStartLogicalSectorNumber(const iso9660_stat_t & statistic) {
	return statistic.lsn;
}

lsn_t CDIOUtilities::getFileStartLogicalSectorNumber(const ISO9660::Stat & statistic) {
	if(statistic.p_stat == nullptr) {
		return 0;
	}

	return getFileStartLogicalSectorNumber(*statistic.p_stat);
}
