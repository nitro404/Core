include_guard()

set(CORE_SOURCE_FILES
	Analytics/Segment/SegmentAnalyticEvent.h
	Analytics/Segment/SegmentAnalyticEvent.cpp
	Analytics/Segment/SegmentAnalytics.h
	Analytics/Segment/SegmentAnalytics.cpp
	Analytics/Segment/SegmentAnalyticsDataStorage.cpp
	Analytics/Segment/SegmentAnalyticsCURL.h
	Analytics/Segment/SegmentAnalyticsCURL.cpp
	Analytics/Segment/SegmentAnalyticsCURLEventTransfer.cpp
	Analytics/Segment/SegmentAnalyticsCURLFailedEvent.cpp
	Application/Application.h
	Application/Application.cpp
	Application/ComponentRegistry.h
	Application/ComponentRegistry.cpp
	Archive/Archive.h
	Archive/Archive.cpp
	Archive/ArchiveEntry.h
	Archive/ArchiveEntry.cpp
	Archive/ArchiveFactoryRegistry.h
	Archive/ArchiveFactoryRegistry.cpp
	Archive/7Zip/SevenZipArchive.h
	Archive/7Zip/SevenZipArchive.cpp
	Archive/7Zip/SevenZipArchiveEntry.cpp
	Archive/Rar/RarArchive.h
	Archive/Rar/RarArchive.cpp
	Archive/Rar/RarArchiveEntry.cpp
	Archive/Tar/CompressedTarArchive.h
	Archive/Tar/CompressedTarArchive.cpp
	Archive/Tar/TarArchive.h
	Archive/Tar/TarArchive.cpp
	Archive/Tar/TarArchiveEntry.cpp
	Archive/Tar/TarBZip2Archive.h
	Archive/Tar/TarBZip2Archive.cpp
	Archive/Tar/TarGZipArchive.h
	Archive/Tar/TarGZipArchive.cpp
	Archive/Tar/TarLZMAArchive.h
	Archive/Tar/TarLZMAArchive.cpp
	Archive/Tar/TarUtilities.h
	Archive/Tar/TarUtilities.cpp
	Archive/Tar/TarXZArchive.h
	Archive/Tar/TarXZArchive.cpp
	Archive/Tar/TarZStandardArchive.h
	Archive/Tar/TarZStandardArchive.cpp
	Archive/Zip/ZipArchive.h
	Archive/Zip/ZipArchive.cpp
	Archive/Zip/ZipArchiveEntry.cpp
	Archive/Zip/ZipArchiveSourceBuffer.cpp
	Archive/Zip/ZipUtilities.h
	Archive/Zip/ZipUtilities.cpp
	Arguments/ArgumentCollection.h
	Arguments/ArgumentCollection.cpp
	Arguments/ArgumentParser.h
	Arguments/ArgumentParser.cpp
	Bitbucket/BitbucketDownload.h
	Bitbucket/BitbucketDownload.cpp
	Bitbucket/BitbucketDownloadCollection.h
	Bitbucket/BitbucketDownloadCollection.cpp
	Bitbucket/BitbucketService.h
	Bitbucket/BitbucketService.cpp
	BitmaskOperators.h
	ByteBuffer.h
	ByteBuffer.cpp
	Colour.h
	Colour.cpp
	Compression/BZip2Utilities.h
	Compression/BZip2Utilities.cpp
	Compression/LZMAUtilities.h
	Compression/LZMAUtilities.cpp
	Compression/ZLibUtilities.h
	Compression/ZLibUtilities.cpp
	Core.h
	Date.h
	Date.cpp
	Dimension.h
	Dimension.cpp
	Endianness.h
	Endianness.cpp
	Factory/Factory.h
	Factory/Factory.cpp
	Factory/FactoryRegistry.h
	Factory/FactoryRegistry.cpp
	GitHub/GitHubRelease.h
	GitHub/GitHubRelease.cpp
	GitHub/GitHubReleaseAsset.h
	GitHub/GitHubReleaseAsset.cpp
	GitHub/GitHubReleaseCollection.h
	GitHub/GitHubReleaseCollection.cpp
	GitHub/GitHubService.h
	GitHub/GitHubService.cpp
	Point2D.h
	Point2D.cpp
	Point3D.h
	Point3D.cpp
	Rectangle.h
	Rectangle.cpp
	LibraryInformation.h
	LibraryInformation.cpp
	Location/FreeGeoIPGeoLocationService.h
	Location/FreeGeoIPGeoLocationService.cpp
	Location/GeoLocation.h
	Location/GeoLocationService.h
	Location/GeoLocationService.cpp
	Logging/LogSystem.h
	Logging/LogSystem.cpp
	Math/ExtendedMath.h
	Math/ExtendedMath.cpp
	Math/Matrix2x2.h
	Math/Matrix2x2.cpp
	Math/Matrix3x3.h
	Math/Matrix3x3.cpp
	Math/Matrix4x4.h
	Math/Matrix4x4.cpp
	Math/Vector2.h
	Math/Vector2.cpp
	Math/Vector3.h
	Math/Vector3.cpp
	Math/Vector4.h
	Math/Vector4.cpp
	Network/HTTPConfiguration.h
	Network/HTTPHeaders.h
	Network/HTTPHeaders.cpp
	Network/HTTPRequest.h
	Network/HTTPRequest.cpp
	Network/HTTPRequestSettings.h
	Network/HTTPRequestSettings.cpp
	Network/HTTPResponse.h
	Network/HTTPResponse.cpp
	Network/HTTPService.h
	Network/HTTPService.cpp
	Network/HTTPTransfer.h
	Network/HTTPTransfer.cpp
	Network/HTTPUtilities.h
	Network/HTTPUtilities.cpp
	Network/IPAddressService.h
	Network/IPAddressService.cpp
	Network/IpifyIPAddressService.h
	Network/IpifyIPAddressService.cpp
	Platform/DeviceInformationBridge.h
	Platform/DeviceInformationBridge.cpp
	Platform/Process.h
	Platform/Process.cpp
	Platform/ProcessCreator.h
	Platform/ProcessCreator.cpp
	Platform/TimeZoneDataManager.h
	Platform/TimeZoneDataManager.cpp
	Script/Script.h
	Script/Script.cpp
	Script/ScriptArguments.h
	Script/ScriptArguments.cpp
	Signal/SignalConnectionGroup.h
	Signal/SignalConnectionGroup.cpp
	Singleton/Singleton.h
	Singleton/SingletonManager.h
	Singleton/SingletonManager.cpp
	Utilities/FileUtilities.h
	Utilities/FileUtilities.cpp
	Utilities/NumberUtilities.h
	Utilities/NumberUtilities.cpp
	Utilities/RapidJSONUtilities.h
	Utilities/RapidJSONUtilities.cpp
	Utilities/StringUtilities.h
	Utilities/StringUtilities.cpp
	Utilities/ThreadUtilities.h
	Utilities/TidyHTMLUtilities.h
	Utilities/TidyHTMLUtilities.cpp
	Utilities/TimeUtilities.h
	Utilities/TimeUtilities.cpp
	Utilities/TinyXML2Utilities.h
	Utilities/TinyXML2Utilities.cpp
	Utilities/Utilities.h
	Utilities/Utilities.cpp
	XML/CustomIndentationXMLPrinter.h
	XML/CustomIndentationXMLPrinter.cpp
)

set(CORE_SOURCE_FILES_WINDOWS
	Archive/NSIS/ArchiveBufferInputStream.h
	Archive/NSIS/ArchiveBufferInputStream.cpp
	Archive/NSIS/ArchiveExtractFileCallback.h
	Archive/NSIS/ArchiveExtractFileCallback.cpp
	Archive/NSIS/ArchiveFileBufferOutputCallback.h
	Archive/NSIS/ArchiveFileBufferOutputCallback.cpp
	Archive/NSIS/ArchiveFileBufferOutputStream.h
	Archive/NSIS/ArchiveFileBufferOutputStream.cpp
	Archive/NSIS/ArchiveOpenCallback.h
	Archive/NSIS/ArchiveOpenCallback.cpp
	Archive/NSIS/NullsoftScriptableInstallSystemArchive.h
	Archive/NSIS/NullsoftScriptableInstallSystemArchive.cpp
	Archive/NSIS/NullsoftScriptableInstallSystemArchiveEntry.cpp
	Archive/Windows/ArchiveFactoryRegistryWindows.cpp
	Factory/Windows/FactoryRegistryWindows.cpp
	Logging/Windows/LogSinkWindows.h
	Logging/Windows/LogSinkWindows.cpp
	Logging/Windows/LogSystemWindows.h
	Logging/Windows/LogSystemWindows.cpp
	Platform/Windows/DeviceInformationBridgeWindows.h
	Platform/Windows/DeviceInformationBridgeWindows.cpp
	Platform/Windows/ProcessCreatorWindows.h
	Platform/Windows/ProcessCreatorWindows.cpp
	Platform/Windows/ProcessWindows.h
	Platform/Windows/ProcessWindows.cpp
	Platform/Windows/TimeZoneDataManagerWindows.h
	Platform/Windows/TimeZoneDataManagerWindows.cpp
	Platform/Windows/WindowsUtilities.h
	Platform/Windows/WindowsUtilities.cpp
	Utilities/Windows/ThreadUtilitiesWindows.cpp
	Utilities/Windows/TimeUtilitiesWindows.cpp
)

list(APPEND CORE_SOURCE_FILES ${CORE_SOURCE_FILES_${PLATFORM_UPPER}})

list(TRANSFORM CORE_SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_SOURCE_FILES})
