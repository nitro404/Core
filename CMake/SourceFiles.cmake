set(CORE_SOURCE_FILES
	Analytics/Segment/SegmentAnalyticEvent.cpp
	Analytics/Segment/SegmentAnalytics.cpp
	Analytics/Segment/SegmentAnalyticsDataStorage.cpp
	Analytics/Segment/SegmentAnalyticsCURL.cpp
	Analytics/Segment/SegmentAnalyticsCURLEventTransfer.cpp
	Analytics/Segment/SegmentAnalyticsCURLFailedEvent.cpp
	Application/Application.cpp
	Application/ComponentRegistry.cpp
	Archive/Archive.cpp
	Archive/ArchiveEntry.cpp
	Archive/ArchiveFactoryRegistry.cpp
	Archive/7Zip/SevenZipArchive.cpp
	Archive/7Zip/SevenZipArchiveEntry.cpp
	Archive/Rar/RarArchive.cpp
	Archive/Rar/RarArchiveEntry.cpp
	Archive/Tar/CompressedTarArchive.cpp
	Archive/Tar/TarArchive.cpp
	Archive/Tar/TarArchiveEntry.cpp
	Archive/Tar/TarBZip2Archive.cpp
	Archive/Tar/TarGZipArchive.cpp
	Archive/Tar/TarLZMAArchive.cpp
	Archive/Tar/TarUtilities.cpp
	Archive/Tar/TarXZArchive.cpp
	Archive/Zip/ZipArchive.cpp
	Archive/Zip/ZipArchiveEntry.cpp
	Archive/Zip/ZipArchiveSourceBuffer.cpp
	Archive/Zip/ZipUtilities.cpp
	Arguments/ArgumentCollection.cpp
	Arguments/ArgumentParser.cpp
	ByteBuffer.cpp
	Colour.cpp
	Compression/BZip2Utilities.cpp
	Compression/LZMAUtilities.cpp
	Compression/ZLibUtilities.cpp
	Date.cpp
	Dimension.cpp
	Endianness.cpp
	Factory/Factory.cpp
	Factory/FactoryRegistry.cpp
	GitHub/GitHubRelease.cpp
	GitHub/GitHubReleaseAsset.cpp
	GitHub/GitHubReleaseCollection.cpp
	GitHub/GitHubService.cpp
	Point.cpp
	Rectangle.cpp
	Location/FreeGeoIPGeoLocationService.cpp
	Location/GeoLocationService.cpp
	Logging/LogSystem.cpp
	Math/ExtendedMath.cpp
	Math/Matrix2x2.cpp
	Math/Matrix3x3.cpp
	Math/Matrix4x4.cpp
	Math/Vector2.cpp
	Math/Vector3.cpp
	Math/Vector4.cpp
	Network/HTTPHeaders.cpp
	Network/HTTPRequest.cpp
	Network/HTTPRequestSettings.cpp
	Network/HTTPResponse.cpp
	Network/HTTPService.cpp
	Network/HTTPTransfer.cpp
	Network/HTTPUtilities.cpp
	Network/IPAddressService.cpp
	Network/IpifyIPAddressService.cpp
	Platform/DeviceInformationBridge.cpp
	Platform/Process.cpp
	Platform/ProcessCreator.cpp
	Script/Script.cpp
	Script/ScriptArguments.cpp
	Singleton/SingletonManager.cpp
	Utilities/FileUtilities.cpp
	Utilities/NumberUtilities.cpp
	Utilities/RapidJSONUtilities.cpp
	Utilities/StringUtilities.cpp
	Utilities/TidyHTMLUtilities.cpp
	Utilities/TimeUtilities.cpp
	Utilities/TinyXML2Utilities.cpp
	Utilities/Utilities.cpp
)

set(CORE_HEADER_FILES
	Analytics/Segment/SegmentAnalyticEvent.h
	Analytics/Segment/SegmentAnalytics.h
	Analytics/Segment/SegmentAnalyticsCURL.h
	Application/Application.h
	Application/ComponentRegistry.h
	Archive/Archive.h
	Archive/ArchiveEntry.h
	Archive/ArchiveFactoryRegistry.h
	Archive/7Zip/SevenZipArchive.h
	Archive/Rar/RarArchive.h
	Archive/Tar/CompressedTarArchive.h
	Archive/Tar/TarArchive.h
	Archive/Tar/TarBZip2Archive.h
	Archive/Tar/TarGZipArchive.h
	Archive/Tar/TarLZMAArchive.h
	Archive/Tar/TarUtilities.h
	Archive/Tar/TarXZArchive.h
	Archive/Zip/ZipArchive.h
	Archive/Zip/ZipUtilities.h
	Arguments/ArgumentCollection.h
	Arguments/ArgumentParser.h
	BitmaskOperators.h
	ByteBuffer.h
	Colour.h
	Compression/BZip2Utilities.h
	Compression/LZMAUtilities.h
	Compression/ZLibUtilities.h
	Core.h
	Date.h
	Dimension.h
	Endianness.h
	Factory/Factory.h
	Factory/FactoryRegistry.h
	Point.h
	Rectangle.h
	Location/FreeGeoIPGeoLocationService.h
	Location/GeoLocation.h
	Location/GeoLocationService.h
	Logging/LogSystem.h
	Math/ExtendedMath.h
	Math/Matrix2x2.h
	Math/Matrix3x3.h
	Math/Matrix4x4.h
	Math/Vector2.h
	Math/Vector3.h
	Math/Vector4.h
	Network/HTTPConfiguration.h
	Network/HTTPHeaders.h
	Network/HTTPRequest.h
	Network/HTTPRequestSettings.h
	Network/HTTPResponse.h
	Network/HTTPService.h
	Network/HTTPTransfer.h
	Network/HTTPUtilities.h
	Network/IPAddressService.h
	Network/IpifyIPAddressService.h
	Platform/DeviceInformationBridge.h
	Platform/Process.h
	Platform/ProcessCreator.h
	Script/Script.h
	Script/ScriptArguments.h
	Singleton/Singleton.h
	Singleton/SingletonManager.h
	Utilities/FileUtilities.h
	Utilities/NumberUtilities.h
	Utilities/RapidJSONUtilities.h
	Utilities/StringUtilities.h
	Utilities/TidyHTMLUtilities.h
	Utilities/TimeUtilities.h
	Utilities/TinyXML2Utilities.h
	Utilities/Utilities.h
)

set(CORE_SOURCE_FILES_WINDOWS
	Factory/Windows/FactoryRegistryWindows.cpp
	Logging/Windows/LogSinkWindows.cpp
	Logging/Windows/LogSystemWindows.cpp
	Platform/Windows/DeviceInformationBridgeWindows.cpp
	Platform/Windows/ProcessCreatorWindows.cpp
	Platform/Windows/ProcessWindows.cpp
	Platform/Windows/WindowsUtilities.cpp
	Utilities/Windows/TimeUtilitiesWindows.cpp
)

set(CORE_HEADER_FILES_WINDOWS
	Logging/Windows/LogSinkWindows.h
	Logging/Windows/LogSystemWindows.h
	Platform/Windows/DeviceInformationBridgeWindows.h
	Platform/Windows/ProcessCreatorWindows.h
	Platform/Windows/ProcessWindows.h
	Platform/Windows/WindowsUtilities.h
)

list(APPEND CORE_HEADER_FILES ${CORE_HEADER_FILES_${PLATFORM_UPPER}})
list(APPEND CORE_SOURCE_FILES ${CORE_SOURCE_FILES_${PLATFORM_UPPER}})

list(TRANSFORM CORE_HEADER_FILES PREPEND "${_SOURCE_DIRECTORY}/")
list(TRANSFORM CORE_SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

list(APPEND CORE_ALL_FILES ${CORE_HEADER_FILES} ${CORE_SOURCE_FILES} )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_ALL_FILES})
