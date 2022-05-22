set(SOURCE_FILES
	Analytics/Segment/SegmentAnalyticEvent.cpp
	Analytics/Segment/SegmentAnalytics.cpp
	Analytics/Segment/SegmentAnalyticsDataStorage.cpp
	Analytics/Segment/SegmentAnalyticsCURL.cpp
	Analytics/Segment/SegmentAnalyticsCURLEventTransfer.cpp
	Analytics/Segment/SegmentAnalyticsCURLFailedEvent.cpp
	Application/Application.cpp
	Application/ComponentRegistry.cpp
	Arguments/ArgumentCollection.cpp
	Arguments/ArgumentParser.cpp
	ByteBuffer.cpp
	Colour.cpp
	Date.cpp
	Dimension.cpp
	Endianness.cpp
	Factory/Factory.cpp
	Factory/FactoryRegistry.cpp
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
	Network/HTTPResponse.cpp
	Network/HTTPService.cpp
	Network/HTTPTimeout.cpp
	Network/HTTPTransfer.cpp
	Network/HTTPUtilities.cpp
	Network/IPAddressService.cpp
	Network/IpifyIPAddressService.cpp
	Platform/DeviceInformationBridge.cpp
	Script/Script.cpp
	Script/ScriptArguments.cpp
	Singleton/SingletonManager.cpp
	Utilities/FileUtilities.cpp
	Utilities/NumberUtilities.cpp
	Utilities/RapidJSONUtilities.cpp
	Utilities/StringUtilities.cpp
	Utilities/TimeUtilities.cpp
	Utilities/Utilities.cpp
	Zip/ZipArchive.cpp
	Zip/ZipArchiveEntry.cpp
	Zip/ZipArchiveSourceBuffer.cpp
	Zip/ZipUtilities.cpp
)

set(SOURCE_FILES_WINDOWS
	Factory/Windows/FactoryRegistryWindows.cpp
	Logging/Windows/LogSinkWindows.cpp
	Logging/Windows/LogSystemWindows.cpp
	Platform/Windows/DeviceInformationBridgeWindows.cpp
	Platform/Windows/WindowsUtilities.cpp
	Utilities/Windows/TimeUtilitiesWindows.cpp
)

source_group(Source                     REGULAR_EXPRESSION ".*\\.(h|cpp)")
source_group(Source\\Analytics          REGULAR_EXPRESSION "Analytics/.*\\.(h|cpp)")
source_group(Source\\Analytics\\Segment REGULAR_EXPRESSION "Analytics/Segment/.*\\.(h|cpp)")
source_group(Source\\Application        REGULAR_EXPRESSION "Application/.*\\.(h|cpp)")
source_group(Source\\Arguments          REGULAR_EXPRESSION "Arguments/.*\\.(h|cpp)")
source_group(Source\\Factory            REGULAR_EXPRESSION "Factory/.*\\.(h|cpp)")
source_group(Source\\Factory\\Windows   REGULAR_EXPRESSION "Factory/Windows/.*\\.(h|cpp)")
source_group(Source\\Location           REGULAR_EXPRESSION "Location/.*\\.(h|cpp)")
source_group(Source\\Logging            REGULAR_EXPRESSION "Logging/.*\\.(h|cpp)")
source_group(Source\\Logging\\Windows   REGULAR_EXPRESSION "Logging/Windows.*\\.(h|cpp)")
source_group(Source\\Math               REGULAR_EXPRESSION "Math/.*\\.(h|cpp)")
source_group(Source\\Network            REGULAR_EXPRESSION "Network/.*\\.(h|cpp)")
source_group(Source\\Platform           REGULAR_EXPRESSION "Platform/.*\\.(h|cpp)")
source_group(Source\\Platform\\Windows  REGULAR_EXPRESSION "Platform/Windows/.*\\.(h|cpp)")
source_group(Source\\Script             REGULAR_EXPRESSION "Script/.*\\.(h|cpp)")
source_group(Source\\Singleton          REGULAR_EXPRESSION "Singleton/.*\\.(h|cpp)")
source_group(Source\\Utilities          REGULAR_EXPRESSION "Utilities/.*\\.(h|cpp)")
source_group(Source\\Zip                REGULAR_EXPRESSION "Zip/.*\\.(h|cpp)")
