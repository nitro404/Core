set(SOURCE_FILES
	ByteBuffer.cpp
	Colour.cpp
	Date.cpp
	Dimension.cpp
	Endianness.cpp
	Point.cpp
	Rectangle.cpp
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
	Platform/DeviceInformationBridge.cpp
	Platform/PlatformBridgeFactory.cpp
	Platform/PlatformBridgeLocator.cpp
	Utilities/FileUtilities.cpp
	Utilities/NumberUtilities.cpp
	Utilities/RapidJSONUtilities.cpp
	Utilities/StringUtilities.cpp
	Utilities/TimeUtilities.cpp
	Utilities/Utilities.cpp
)

set(SOURCE_FILES_WINDOWS
	Platform/Windows/DeviceInformationBridgeWindows.cpp
	Platform/Windows/WindowsUtilities.cpp
	Utilities/Windows/TimeUtilitiesWindows.cpp
)

source_group(Source                    REGULAR_EXPRESSION ".*\\.(h|cpp)")
source_group(Source\\Math              REGULAR_EXPRESSION "Math/.*\\.(h|cpp)")
source_group(Source\\Network           REGULAR_EXPRESSION "Network/.*\\.(h|cpp)")
source_group(Source\\Platform          REGULAR_EXPRESSION "Platform/.*\\.(h|cpp)")
source_group(Source\\Platform\\Windows REGULAR_EXPRESSION "Platform/Windows/.*\\.(h|cpp)")
source_group(Source\\Utilities         REGULAR_EXPRESSION "Utilities/.*\\.(h|cpp)")
