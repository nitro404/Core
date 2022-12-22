#include "NullsoftScriptableInstallSystemArchive.h"

#include "BitmaskOperators.h"

#include "Utilities/FileUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/TimeUtilities.h"

#include <spdlog/spdlog.h>
#include <magic_enum.hpp>

#include <array>
#include <filesystem>
#include <sstream>

static constexpr std::array<uint8_t, 16> NSIS_SIGNATURE = { 0xEF, 0xBE, 0xAD, 0xDE, 'N', 'u', 'l', 'l', 's', 'o', 'f', 't', 'I', 'n', 's', 't' };
static constexpr uint8_t NUMBER_OF_COMMAND_PARAMETERS = 6;
static constexpr uint8_t COMMAND_SIZE = sizeof(uint32_t) + (NUMBER_OF_COMMAND_PARAMETERS * sizeof(uint32_t));
static constexpr uint16_t PARK_CODE = 0xE001;
static constexpr uint8_t NSIS_CODE_VARIABLE = 253;
static constexpr uint8_t NSIS3_CODE_VARIABLE = 3;
static constexpr uint16_t PARK_NUMBER_MASK = 0x7FFF;
static constexpr uint8_t NSIS_OUTPUT_DIRECTORY = 22;
static constexpr uint8_t NSIS_225_SPECIAL_OUTPUT_DIRECTORY = 29; // NSIS 2.04 - 2.25
static constexpr uint8_t NSIS_226_SPECIAL_OUTPUT_DIRECTORY = 31; // NSIS 2.26+

enum class MethodType : uint8_t {
	Copy,
	Deflate,
	BZip2,
	LZMA
};

enum class Type : uint8_t {
	NSIS2,
	NSIS3,
	Park1, // Park 2.46.1
	Park2, // Park 2.46.2  (GetFontVersion)
	Park3  // Park 2.46.3+ (GetFontName)
};

enum class Command : uint8_t {
	InvalidOperationCode,
	Return,                       // Return
	NoOperation,                  // Nop, Goto
	Abort,                        // Abort
	Quit,                         // Quit
	Call,                         // Call, InitPluginsDir
	UpdateText,                    // DetailPrint
	BringToFront,                 // BringToFront
	SetDetailsView,               // SetDetailsView
	SetFileAttributes,            // SetFileAttributes
	CreateDirectory,              // CreateDirectory, SetOutPath
	IfFileExists,                 // IfFileExists
	SetRebootFlag,                // SetRebootFlag, ...
	IfFlag,                       // IfAbort, IfSilent, IfErrors, IfRebootFlag
	GetFlag,                      // GetInstDirError, GetErrorLevel
	Rename,                       // Rename
	GetFullPathName,              // GetFullPathName
	SearchPath,                   // SearchPath
	GetTemporaryFileName,         // GetTempFileName
	ExtractFile,                  // File
	DeleteFile,                   // Delete
	MessageBox,                   // MessageBox
	RemvoeDirectory,              // RMDir
	StringLength,                 // StrLen
	AssignVariable,               // StrCpy
	StringCompare,                // StrCmp
	ReadEnvironmentString,        // ReadEnvStr, ExpandEnvStrings
	IntegerCompare,               // IntCmp, IntCmpU
	IntegerOperation,             // IntOp
	IntegerFormat,                // IntFmt/Int64Fmt
	PushPop,                      // Push/Pop/Exchange
	FindWindow,                   // FindWindow
	SendMessage,                  // SendMessage
	IsWindow,                     // IsWindow
	GetDialogItem,                // GetDlgItem
	SetControlColors,             // SetCtlColors
	SetBrandingImage,             // SetBrandingImage / LoadAndSetImage
	CreateFont,                   // CreateFont
	ShowWindow,                   // ShowWindow, EnableWindow, HideWindow
	ExecuteShell,                 // ExecShell
	Execute,                      // Exec, ExecWait
	GetFileTime,                  // GetFileTime
	GetDynamicLinkLibraryVersion, // GetDLLVersion
	// GetFontVersion,            // Park 2.46.2
	// GetFontName,               // Park 2.46.3
	RegisterDynamicLinkLibrary,   // RegDLL, UnRegDLL, CallInstDLL
	CreateShortcut,               // CreateShortCut
	CopyFiles,                    // CopyFiles
	Reboot,                       // Reboot
	WriteINI,                     // WriteINIStr, DeleteINISec, DeleteINIStr, FlushINI
	ReadINIString,                // ReadINIStr
	DeleteRegistry,               // DeleteRegValue, DeleteRegKey
	WriteRegistry,                // WriteRegStr, WriteRegExpandStr, WriteRegBin, WriteRegDWORD
	ReadRegistryString,           // ReadRegStr, ReadRegDWORD
	RegistryEnumeration,          // EnumRegKey, EnumRegValue
	FileClose,                    // FileClose
	FileOpen,                     // FileOpen
	FileWrite,                    // FileWrite, FileWriteByte
	FileRead,                     // FileRead, FileReadByte
	// Park:
	// FileWriteWideString,       // FileWriteUTF16LE, FileWriteWord
	// FileReadWideString,        // FileReadUTF16LE, FileReadWord
	FileSeek,                     // FileSeek
	FindClose,                    // FindClose
	FindNext,                     // FindNext
	FindFirst,                    // FindFirst
	WriteUninstaller,             // WriteUninstaller
	// Park : since 2.46.3 the log is enabled in main Park version
	// Log,                       // LogSet, LogText
	SectionSet,                   // Get*, Set*
	InstallTypeSet,               // InstTypeSetText, InstTypeGetText, SetCurInstType, GetCurInstType
	/*
	// before NSIS v3.06 it was so:
	// instructions not actually implemented in exehead, but used in compiler.
	GetLabeLAddress,              // both of these get converted to AssignVariable
	GetFunctionAddress,
	*/
	// v3.06 and later it was changed to:
	GetOperatingSystemInformation,
	ReservedOperationCode,
	LockWindow,                   // LockWindow
	// 2 unicode commands available only in Unicode archive
	FileWriteWideString,          // FileWriteUTF16LE, FileWriteWord
	FileReadWideString,           // FileReadUTF16LE, FileReadWord
	/*
	// since v3.06 the fllowing IDs codes was moved here:
	// Opcodes listed here are not actually used in exehead. No exehead opcodes should be present after these!
	GetLabelAddress,              // --> AssignVariable
	GetFunctionAddress,           // --> AssignVariable
	*/
	// The following IDs are not IDs in real order.
	// We just need some IDs to translate eny extended layout to main layout.
	Log,                          // LogSet, LogText
	// Park
	FindProcess,                  // FindProc
	GetFontVersion,               // GetFontVersion
	GetFontName,                  // GetFontName
};

struct VersionInformation final {
	Type type = Type::NSIS2;
	bool isNSIS200 = false;
	bool isNSIS225 = false;
};

struct FirstHeader final {
	enum class Flag : uint8_t {
		None = 0,
		Uninstaller = 1,
		Silent = 1 << 1,
		NoCRC = 1 << 2,
		ForceCRC = 1 << 3,
		BigInstallerLongOffset = 1 << 4,
		BigInstallerExternalFileSupport = 1 << 5,
		BigInstallerExternalFile = 1 << 6,
		BigInstallerStubInstaller = 1 << 7
	};

	Flag flags = Flag::None;
	uint32_t headerSize = 0;
	uint32_t dataSize = 0;
};

template<>
struct BitmaskOperators<FirstHeader::Flag> {
	static const bool enabled = true;
};

static const uint8_t COMMAND_PARAMETER_COUNTS[] = {
	0, // "Invalid"
	0, // Return
	1, // Nop, Goto
	1, // "Abort"
	0, // "Quit"
	2, // Call
	6, // "DetailPrint" // 1 param in new versions, 6 in old NSIS versions
	1, // "Sleep"
	0, // "BringToFront"
	2, // "SetDetailsView"
	2, // "SetFileAttributes"
	3, // CreateDirectory, SetOutPath
	3, // "IfFileExists"
	3, // SetRebootFlag, ...
	4, // "If" // IfAbort, IfSilent, IfErrors, IfRebootFlag
	2, // "Get" // GetInstDirError, GetErrorLevel
	4, // "Rename"
	3, // "GetFullPathName"
	2, // "SearchPath"
	2, // "GetTempFileName"
	6, // "File"
	2, // "Delete"
	6, // "MessageBox"
	2, // "RMDir"
	2, // "StrLen"
	4, // StrCpy, GetCurrentAddress
	5, // "StrCmp"
	3, // ReadEnvStr, ExpandEnvStrings
	6, // "IntCmp"
	4, // "IntOp"
	4, // "IntFmt" EW_INTFMT
	6, // Push, Pop, Exch // it must be 3 params. But some multi-command write garbage.
	5, // "FindWindow"
	6, // "SendMessage"
	3, // "IsWindow"
	3, // "GetDlgItem"
	2, // "SetCtlColors"
	4, // "SetBrandingImage" // LoadAndSetImage
	5, // "CreateFont"
	4, // ShowWindow, EnableWindow, HideWindow
	6, // "ExecShell"
	3, // "Exec" // Exec, ExecWait
	3, // "GetFileTime"
	4, // "GetDLLVersion"
	6, // RegDLL, UnRegDLL, CallInstDLL // it must be 5 params. But some multi-command write garbage.
	6, // "CreateShortCut"
	4, // "CopyFiles"
	1, // "Reboot"
	5, // WriteINIStr, DeleteINISec, DeleteINIStr, FlushINI
	4, // "ReadINIStr"
	5, // "DeleteReg" // DeleteRegKey, DeleteRegValue
	6, // "WriteReg"  // WriteRegStr, WriteRegExpandStr, WriteRegBin, WriteRegDWORD
	5, // "ReadReg" // ReadRegStr, ReadRegDWORD
	5, // "EnumReg" // EnumRegKey, EnumRegValue
	1, // "FileClose"
	4, // "FileOpen"
	3, // "FileWrite" // FileWrite, FileWriteByte
	4, // "FileRead" // FileRead, FileReadByte
	4, // "FileSeek"
	1, // "FindClose"
	2, // "FindNext"
	3, // "FindFirst"
	4, // "WriteUninstaller"
	5, // "Section"  // ***
	4, // InstTypeSetText, InstTypeGetText, SetCurInstType, GetCurInstType
	// 6 // "GetLabelAddr" // before 3.06
	6, // "GetOsInfo" GetKnownFolderPath, ReadMemory, // v3.06+
	2, // "GetFunctionAddress" // before 3.06
	1, // "LockWindow"
	4, // "FileWrite" // FileWriteUTF16LE, FileWriteWord
	4, // "FileRead" // FileReadUTF16LE, FileReadWord
	2, // "Log" // LogSet, LogText
	// Park
	2, // "FindProc"
	2, // "GetFontVersion"
	2, // "GetFontName"
};

static std::optional<ByteBuffer::CompressionMethod> methodTypeToCompressionMethod(MethodType methodType) {
	switch(methodType) {
		case MethodType::Copy:
			break;

		case MethodType::Deflate:
			return ByteBuffer::CompressionMethod::ZLib;

		case MethodType::BZip2:
			return ByteBuffer::CompressionMethod::BZip2;

		case MethodType::LZMA:
			return ByteBuffer::CompressionMethod::LZMA;
	}

	return {};
}

static bool isBZip2(const ByteBuffer & data) {
	if(!data.canReadBytes(2)) {
		return false;
	}

	return data[data.getReadOffset()    ] == 0x31 &&
	       data[data.getReadOffset() + 1] < 14;
}

static bool isLZMA(const ByteBuffer & data, uint32_t & dictionarySize, size_t readOffset) {
	if(!data.canReadBytes(7)) {
		return false;
	}

	std::optional<uint32_t> optionalDictionarySize(data.getUnsignedInteger(readOffset + 1));

	if(!optionalDictionarySize.has_value()) {
		return false;
	}

	dictionarySize = optionalDictionarySize.value();

	return  data[readOffset    ]         == 0x5D &&
	        data[readOffset + 1]         == 0x00 &&
	        data[readOffset + 2]         == 0x00 &&
	        data[readOffset + 5]         == 0x00 &&
	       (data[readOffset + 6] & 0x80) == 0x00;

}

static bool isLZMA(const ByteBuffer & data, uint32_t & dictionarySize, bool & filterFlag) {
// TODO: temp:
//spdlog::info("lzma check index {}", data.getReadOffset());

	if(isLZMA(data, dictionarySize, data.getReadOffset())) {
		filterFlag = true;
		return true;
	}

	if(!data.canReadBytes(2)) {
		return false;
	}

	size_t readOffset = data.getReadOffset();

	if(data[readOffset] <= 1 && isLZMA(data, dictionarySize, data.getReadOffset() + 1)) {
		filterFlag = true;
		return true;
	}

	return false;
}

NullsoftScriptableInstallSystemArchive::NullsoftScriptableInstallSystemArchive(const std::string & filePath)
	: m_filePath(filePath)
	, m_numberOfFiles(0)
	, m_numberOfDirectories(0) { }

NullsoftScriptableInstallSystemArchive::~NullsoftScriptableInstallSystemArchive() {
	for(std::vector<std::shared_ptr<Entry>>::iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		(*i)->clearParentArchive();
	}
}

std::string NullsoftScriptableInstallSystemArchive::getFilePath() const {
	return m_filePath;
}

uint64_t NullsoftScriptableInstallSystemArchive::getInflatedSize() const {
	uint64_t inflatedSize = 0;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		inflatedSize += (*i)->getInflatedSize();
	}

	return inflatedSize;
}

size_t NullsoftScriptableInstallSystemArchive::numberOfEntries() const {
	return m_entries.size();
}

size_t NullsoftScriptableInstallSystemArchive::numberOfFiles() const {
	return m_numberOfFiles;
}

size_t NullsoftScriptableInstallSystemArchive::numberOfDirectories() const {
	return m_numberOfDirectories;
}

bool NullsoftScriptableInstallSystemArchive::hasEntry(const Entry & entry) const {
	return entry.getParentArchive() == this &&
			 entry.getIndex() < m_entries.size() &&
			 m_entries[entry.getIndex()] != nullptr &&
			 m_entries[entry.getIndex()].get() == &entry;
}

bool NullsoftScriptableInstallSystemArchive::hasEntry(const std::string & entryPath, bool caseSensitive) const {
	return indexOfEntry(entryPath, caseSensitive) != std::numeric_limits<size_t>::max();
}

bool NullsoftScriptableInstallSystemArchive::hasEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive) != std::numeric_limits<size_t>::max();
}

size_t NullsoftScriptableInstallSystemArchive::indexOfEntry(const std::string & entryPath, bool caseSensitive) const {
	if(entryPath.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getPath()), Utilities::trimTrailingPathSeparator(entryPath), caseSensitive)) {
			return i - m_entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t NullsoftScriptableInstallSystemArchive::indexOfFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	if(entryName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(!includeSubdirectories && (*i)->isInSubdirectory()) {
			continue;
		}

		if(Utilities::areStringsEqual(Utilities::trimTrailingPathSeparator((*i)->getName()), Utilities::trimTrailingPathSeparator(entryName), caseSensitive)) {
			return i - m_entries.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry> NullsoftScriptableInstallSystemArchive::getEntry(const std::string & entryPath, bool caseSensitive) const {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry> NullsoftScriptableInstallSystemArchive::getEntry(const std::string & entryPath, bool caseSensitive) {
	return getEntry(indexOfEntry(entryPath, caseSensitive));
}

std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry> NullsoftScriptableInstallSystemArchive::getFirstEntryWithName(const std::string & entryName, bool includeSubdirectories, bool caseSensitive) const {
	return getEntry(indexOfFirstEntryWithName(entryName, includeSubdirectories, caseSensitive));
}

const std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry> NullsoftScriptableInstallSystemArchive::getEntry(size_t index) const {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

std::weak_ptr<NullsoftScriptableInstallSystemArchive::Entry> NullsoftScriptableInstallSystemArchive::getEntry(size_t index) {
	if(index >= m_entries.size()) {
		return std::weak_ptr<Entry>();
	}

	return m_entries[index];
}

std::string NullsoftScriptableInstallSystemArchive::toDebugString(bool includeDate) const {
	std::stringstream stringStream;

	stringStream << fmt::format("File Path: '{}'\n", m_filePath);
	stringStream << fmt::format("Number of Entries: {} (Files: {}, Directories: {})\n", numberOfEntries(), m_numberOfFiles, m_numberOfDirectories);

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i) {
		stringStream << fmt::format("{}. '{}' Size: {}", (*i)->getIndex(), (*i)->getPath(), (*i)->getInflatedSize());

		if(includeDate) {
			stringStream << fmt::format(" Date: {}", Utilities::timePointToString((*i)->getDate()));
		}

		stringStream << "\n";
	}

	return stringStream.str();
}

static inline bool checkPeOffset(uint32_t pe) {
	return (pe >= 0x40 && pe <= 0x1000 && (pe & 7) == 0);
}

static bool isArcPe(const ByteBuffer & data, size_t offset) {
	static constexpr size_t PE_START_SIZE = 0x40;
	static constexpr size_t PE_HEADER_SIZE = 4 + 20;
	static constexpr size_t PE_OPT_HEADER32_SIZE_MINIMUM = 96;
	static const std::string PE_MZ_STRING("MZ");

// TODO: temp:
static constexpr bool VERBOSE = false;

	size_t currentOffset = offset;

	if(data.getSize() <= currentOffset + PE_MZ_STRING.size()) {
if(VERBOSE) spdlog::info("missing pe mz string");
		return false;
	}

	std::optional<std::string> optionalPEMZString(data.getString(2, offset));

	if(optionalPEMZString != PE_MZ_STRING) {
if(VERBOSE) spdlog::info("pe mz string doesn't match");
		return false;
	}

	currentOffset += PE_MZ_STRING.size();

	if(data.getSize() <= currentOffset + PE_START_SIZE) {
if(VERBOSE) spdlog::info("data missing pe value");
		return false;
	}

	std::optional<uint32_t> optionalPE = data.getUnsignedInteger(currentOffset + 0x3C);

	if(!optionalPE.has_value()) {
if(VERBOSE) spdlog::info("data pe value parse failed");
		return false;
	}

	uint32_t pe = optionalPE.value();

	if(pe < 0x40 || pe > 0x1000 || (pe & 7) != 0) {
if(VERBOSE) spdlog::info("pe offset check failed");
		return false;
	}

	currentOffset = offset + pe;

	if(data.getSize() <= currentOffset + 4) {
if(VERBOSE) spdlog::info("data missing value");
		return false;
	}

	std::optional<uint32_t> optionalValue(data.getUnsignedInteger(currentOffset));

	if(optionalValue != 0x00004550) {
if(VERBOSE) spdlog::info("data value not 0x00004550");
		return false;
	}

	currentOffset += 4 + 16;

	if(data.getSize() <= currentOffset + 2) {
if(VERBOSE) spdlog::info("data missing pe optional header 32 size");
		return false;
	}

	std::optional<uint16_t> optionalPEOptHeader32Size(data.getUnsignedShort(currentOffset));

	if(!optionalPEOptHeader32Size.has_value()) {
if(VERBOSE) spdlog::info("data missing pe optional header 32 size read failed");
		return false;
	}

	return optionalPEOptHeader32Size.value() >= PE_OPT_HEADER32_SIZE_MINIMUM;
}

struct BlockHeader
{
	uint32_t offset = 0;
	uint32_t num = 0;

	bool parse(const ByteBuffer & data, uint8_t bhoSize);
};

bool BlockHeader::parse(const ByteBuffer & data, uint8_t bhoSize) {
	if(bhoSize == 12) {
		if(data.getUnsignedInteger(data.getReadOffset() + 4) != 0) {
			return false;
		}
	}

	if(!data.canReadBytes(data.getReadOffset() + bhoSize)) {
		return false;
	}

	offset = data.getUnsignedInteger(data.getReadOffset(), nullptr);
	num = data.getUnsignedInteger(data.getReadOffset() + bhoSize - 4, nullptr);

	return true;
}

static Command getConvertedCommand(uint32_t commandID, Type type, bool unicode, bool logCommandEnabled) {
	if(type < Type::Park1) {
		if(!logCommandEnabled) {
			return magic_enum::enum_value<Command>(commandID);
		}

		if(commandID < magic_enum::enum_integer(Command::SectionSet)) {
			return magic_enum::enum_value<Command>(commandID);
		}

		if(commandID == magic_enum::enum_integer(Command::SectionSet)) {
			return Command::Log;
		}

		return magic_enum::enum_value<Command>(commandID - 1);
	}

	if(commandID < magic_enum::enum_integer(Command::RegisterDynamicLinkLibrary)) {
		return magic_enum::enum_value<Command>(commandID);
	}

	if(type >= Type::Park2) {
		if(commandID == magic_enum::enum_integer(Command::RegisterDynamicLinkLibrary)) {
			return Command::GetFontVersion;
		}

		commandID--;
	}

	if(type >= Type::Park3) {
		if(commandID == magic_enum::enum_integer(Command::RegisterDynamicLinkLibrary)) {
			return Command::GetFontName;
		}

		commandID--;
	}

	if(commandID >= magic_enum::enum_integer(Command::FileSeek)) {
		if(unicode) {
			if(commandID == magic_enum::enum_integer(Command::FileSeek)) {
				return Command::FileWriteWideString;
			}

			if(commandID == magic_enum::enum_integer(Command::FileSeek) + 1) {
				return magic_enum::enum_value<Command>(static_cast<uint8_t>(magic_enum::enum_integer(Command::FileWriteWideString)) + 1);
			}

			commandID -= 2;
		}

		if(commandID >= magic_enum::enum_integer(Command::SectionSet) && logCommandEnabled) {
			if(commandID == magic_enum::enum_integer(Command::SectionSet)) {
				return Command::Log;
			}

			return magic_enum::enum_value<Command>(commandID - 1);
		}

		if(commandID == magic_enum::enum_integer(Command::FileWriteWideString)) {
			return Command::FindProcess;
		}
	}

	return magic_enum::enum_value<Command>(commandID);
}

static int32_t GetVarIndex(const ByteBuffer & data, uint32_t stringPosition, Type type, bool unicode, uint32_t stringsOffset, uint32_t numberOfStringCharacters) {
	if(stringPosition >= numberOfStringCharacters) {
		return -1;
	}

	if(unicode) {
		if(numberOfStringCharacters - stringPosition < 3 * 2) {
			return -1;
		}

		std::optional<uint16_t> optionalCode(data.getUnsignedShort(data.getReadOffset() + stringsOffset + (static_cast<size_t>(stringPosition) * 2)));

		if(!optionalCode.has_value()) {
			return -1;
		}

		std::optional<uint32_t> optionalValue(data.getUnsignedShort(data.getReadOffset() + stringsOffset + (static_cast<size_t>(stringPosition) * 2) + sizeof(uint16_t)));

		if(!optionalValue.has_value() || optionalValue == 0) {
			return -1;
		}

		if(type >= Type::Park1) {
			if(optionalCode.value() != PARK_CODE) {
				return -1;
			}

			return static_cast<int32_t>(optionalValue.value() & PARK_NUMBER_MASK);
		}

		// NSIS3
		{
			if(optionalCode.value() != NSIS3_CODE_VARIABLE) {
				return -1;
			}

			return static_cast<int32_t>((optionalValue.value() & 0x7F) | (((optionalValue.value() >> 8) & 0x7F) << 7));
		}
	}

	if(numberOfStringCharacters - stringPosition < 4) {
		return -1;
	}

	std::optional<uint8_t> optionalCode(data.getUnsignedByte(data.getReadOffset() + stringsOffset + stringPosition));

	if(!optionalCode.has_value()) {
		return -1;
	}

	if(type == Type::NSIS3) {
		if(optionalCode.value() != NSIS3_CODE_VARIABLE) {
			return -1;
		}
	}
	else if(optionalCode.value() != NSIS_CODE_VARIABLE) {
		return -1;
	}

	std::optional<uint8_t> optionalValueA(data.getUnsignedByte(data.getReadOffset() + stringsOffset + stringPosition + 1));

	if(!optionalValueA.has_value() || optionalValueA.value() == 0) {
		return -1;
	}

	std::optional<uint8_t> optionalValueB(data.getUnsignedByte(data.getReadOffset() + stringsOffset + stringPosition + 2));

	if(!optionalValueB.has_value() || optionalValueB.value() == 0) {
		return -1;
	}

	return (optionalValueA.value() & 0x7F) | (static_cast<uint32_t>(optionalValueB.value() & 0x7F) << 7);
}

static int32_t GetVarIndex(const ByteBuffer & data, uint32_t stringPosition, Type type, bool unicode, uint32_t stringsOffset, uint32_t numberOfStringCharacters, uint32_t * offset) {
// TODO: implement me later:
return -1;

/*
	if(offset != nullptr) {
		*offset = 0;
	}

	int32_t variableIndex = GetVarIndex(data, stringPosition, type, unicode, stringsOffset, numberOfStringCharacters);

	if(variableIndex < 0) {
		return variableIndex;
	}

	if(unicode)
	{
		if(NumStringChars - stringPosition < 2 * 2) {
			return -1;
		}

		if(offset != nullptr) {
			*offset = 2;
		}
	}
	else
	{
		if(NumStringChars - stringPosition < 3) {
			return -1;
		}

		if(offset != nullptr) {
			*offset = 3;
		}
	}

	return variableIndex;
*/
}

static int32_t GetVarIndexFinished(const ByteBuffer & data, uint32_t stringPosition, uint8_t endCharacter, Type type, bool unicode, uint32_t stringsOffset, uint32_t numberOfStringCharacters, uint32_t * offset = nullptr) {
	if(offset != nullptr) {
		*offset = 0;
	}

	int32_t variableIndex = GetVarIndex(data, stringPosition, type, unicode, stringPosition, numberOfStringCharacters);

	if(variableIndex < 0) {
		return variableIndex;
	}

	if(unicode) {
		if(numberOfStringCharacters - stringPosition < 3 * 2) {
			return -1;
		}

		if(data.getUnsignedShort(data.getReadOffset() + stringsOffset + (static_cast<size_t>(stringPosition) * 2) + 4) != endCharacter) {
			return -1;
		}

		if(offset != nullptr) {
			*offset = 3;
		}
	}
	else {
		if(numberOfStringCharacters - stringPosition < 4) {
			return -1;
		}

		if(data.getUnsignedByte(data.getReadOffset() + stringsOffset + stringPosition + 4) != endCharacter) {
			return -1;
		}

		if(offset != nullptr) {
			*offset = 4;
		}
	}

	return variableIndex;
}
static bool isCommandParameterString(const ByteBuffer & data, uint32_t stringPosition, uint8_t index, Type type, bool unicode, uint32_t stringsOffset, uint32_t numberOfStringCharacters) {
	if(index > 0x7FFF) {
		return false;
	}

	return GetVarIndexFinished(data, stringPosition, 0, type, unicode, stringsOffset, numberOfStringCharacters) == index;
}

static std::optional<Command> getBadCommand(const ByteBuffer & data, uint32_t numberOfEntries, Type type, bool unicode, bool logCommandEnabled) {
	std::optional<uint32_t> optionalCommandID;
	Command command = Command::InvalidOperationCode;
	std::optional<Command> badCommand;
	uint8_t commandParameterIndex = std::numeric_limits<uint8_t>::max();
	std::optional<uint32_t> optionalCommandParameter;

	for(uint32_t i = 0; i < numberOfEntries; i++) {
		optionalCommandID = data.getUnsignedInteger(data.getReadOffset() + (static_cast<size_t>(COMMAND_SIZE) * i));

		if(!optionalCommandID.has_value()) {
			spdlog::error("Failed to read bad command ID.");

			return {};
		}

		command = getConvertedCommand(optionalCommandID.value(), type, unicode, logCommandEnabled);

		if(magic_enum::enum_integer(command) >= magic_enum::enum_count<Command>() ||
		   (badCommand.has_value() && magic_enum::enum_integer(badCommand.value()) >= 0 && command >= badCommand.value())) {
			continue;
		}

		if(command == Command::ReservedOperationCode ||
		   (type != Type::NSIS3 && command == Command::GetOperatingSystemInformation)) {
			badCommand = command;
			continue;
		}

		for(commandParameterIndex = NUMBER_OF_COMMAND_PARAMETERS; commandParameterIndex != 0; commandParameterIndex--) {
			optionalCommandParameter = data.getUnsignedInteger(data.getReadOffset() + (commandParameterIndex * sizeof(uint32_t)));

			if(!optionalCommandParameter.has_value()) {
				spdlog::error("Failed to read bad command parameter #{} for entry #{}.", commandParameterIndex + 1, i + 1);

				return {};
			}

			if(optionalCommandParameter.value() != 0) {
				break;
			}
		}

		if(command == Command::FindProcess &&
		   commandParameterIndex == 0) {
			badCommand = command;
			continue;
		}

		if(COMMAND_PARAMETER_COUNTS[magic_enum::enum_integer(command)] < commandParameterIndex) {
			badCommand = command;
		}
	}

	return badCommand;
}

static bool AreTwoParamStringsEqual(const ByteBuffer & data, uint32_t parameterStringA, uint32_t parameterStringB, bool unicode, uint32_t stringsOffset, uint32_t numberOfStringCharacters) {
	if(parameterStringA == parameterStringB) {
		return true;
	}

	// NSIS-3.0a1 probably contains bug, so it can use 2 different strings
	// with same content. So we check real string also.
	// Also it's possible to check identical postfix parts of strings.
	if(parameterStringA >= numberOfStringCharacters ||
	   parameterStringB >= numberOfStringCharacters) {
		return false;
	}

	uint8_t characterSize = unicode ? 2 : 1;
	uint32_t parameterStringAOffset = stringsOffset + (parameterStringA * characterSize);
	uint32_t parameterStringBOffset = stringsOffset + (parameterStringB * characterSize);

	if(unicode) {
		while(true) {
			std::optional<uint16_t> optionalWideCharacter(data.getUnsignedShort(data.getReadOffset() + parameterStringAOffset));

			if(!optionalWideCharacter.has_value()) {
				return false;
			}

			if(optionalWideCharacter != data.getUnsignedShort(data.getReadOffset() + parameterStringBOffset)) {
				return false;
			}

			if(optionalWideCharacter.value() == 0) {
				return true;
			}

			parameterStringAOffset += characterSize;
			parameterStringBOffset += characterSize;
		}
	}
	else {
		while(true) {
			std::optional<uint8_t> optionalCharacter(data.getUnsignedByte(data.getReadOffset() + parameterStringAOffset));

			if(!optionalCharacter.has_value()) {
				return false;
			}

			if(optionalCharacter != data.getUnsignedByte(data.getReadOffset() + parameterStringBOffset)) {
				return false;
			}

			if(optionalCharacter.value() == 0) {
				return true;
			}

			parameterStringAOffset += characterSize;;
			parameterStringBOffset += characterSize;;
		}
	}
}

static VersionInformation detectVersion(const ByteBuffer & data, bool unicode, bool logCommandEnabled, uint32_t numberOfEntries, uint32_t entriesOffset, uint32_t stringsOffset, uint32_t numberOfStringCharacters) {
	static constexpr uint16_t NSIS_3_CODE = 3;
	static constexpr uint8_t NSIS_INSTALL_DIRECTORY = 21;
	static constexpr uint8_t NSIS_225_PARENT_WINDOW_HANDLE = 27;

	size_t initialReadOffset = data.getReadOffset();
	bool isStrongPark = false;
	bool isStrongNSIS = false;
	bool wasParkVersionDetected = false;
	VersionInformation versionInformation;

	if(numberOfStringCharacters > 2) {
		if(!data.skipReadBytes(stringsOffset)) {
			spdlog::error("Failed to skip to string block read offset.");

			return {};
		}

		size_t currentReadOffset = data.getReadOffset();
		uint32_t count = numberOfStringCharacters - 2;

		if(unicode) {
			std::optional<uint16_t> optionalVersion;

			for(uint32_t i = 0; i < count; i++) {
				data.setReadOffset(currentReadOffset + (static_cast<size_t>(i) * 2));

				if(data.readUnsignedShort() == 0) {
					// it can be TXT/RTF with marker char (1 or 2). so we must check next char
					if(data.readUnsignedShort() == NSIS_3_CODE) {
						optionalVersion = data.readUnsignedShort();

						if(optionalVersion.has_value() && (optionalVersion.value() & 0x8080) == 0x8080) {
							versionInformation.type = Type::NSIS3;
							isStrongNSIS = true;
							break;
						}
					}
				}
			}

			if(!isStrongNSIS) {
				versionInformation.type = Type::Park1;
				isStrongPark = true;
			}
		}
		else {
			std::optional<uint8_t> optionalVersion;

			for(uint32_t i = 0; i < count; i++) {
				data.setReadOffset(currentReadOffset + i);

				if(data.readByte() == 0) {
					// it can be TXT/RTF with marker char (1 or 2). so we must check next char
					// for marker=1 (txt)
					if(data.readByte() == NSIS_3_CODE) {
						optionalVersion = data.readByte();

						if(optionalVersion.has_value() && (optionalVersion.value() & 0x80) != 0) {
							versionInformation.type = Type::NSIS3;
							isStrongNSIS = true;
							break;
						}
					}
				}
			}
		}
	}

	data.setReadOffset(initialReadOffset);

	if(versionInformation.type == Type::NSIS2 && !unicode) {
		Command command = Command::InvalidOperationCode;
		std::optional<uint32_t> optionalCommandParameter;
		uint32_t commandParameters[NUMBER_OF_COMMAND_PARAMETERS];

		for(uint32_t i = 0; i < numberOfEntries; i++) {
			std::optional<uint32_t> optionalCommandID(data.readUnsignedInteger());

			if(!optionalCommandID.has_value()) {
				spdlog::error("Failed to read command ID.");

				return {};
			}

			command = getConvertedCommand(optionalCommandID.value(), versionInformation.type, unicode, logCommandEnabled);

			if(command != Command::GetDialogItem && command != Command::AssignVariable) {
				data.skipReadBytes(NUMBER_OF_COMMAND_PARAMETERS * sizeof(uint32_t));

				continue;
			}

			for(uint8_t j = 0; j < NUMBER_OF_COMMAND_PARAMETERS; j++) {
				optionalCommandParameter = data.readUnsignedInteger();

				if(!optionalCommandParameter.has_value()) {
					spdlog::error("Failed to read command parameter #{} for entry #{}.", j + 1, i + 1);

					return {};
				}

				commandParameters[j] = optionalCommandParameter.value();
			}

			if(command == Command::GetDialogItem) {
				// we can also use Command::SetControlColors
// TODO: must be called with 0 read offset or otherwise updated:
				if(isCommandParameterString(data, commandParameters[1], NSIS_225_PARENT_WINDOW_HANDLE, versionInformation.type, unicode, stringsOffset, numberOfStringCharacters)) {
					versionInformation.isNSIS225 = true;

					if(commandParameters[0] == NSIS_225_SPECIAL_OUTPUT_DIRECTORY) {
						versionInformation.isNSIS200 = true;
						break;
					}
				}
			}
			else if(command == Command::AssignVariable) {
				if(commandParameters[0] == NSIS_225_SPECIAL_OUTPUT_DIRECTORY &&
				   commandParameters[2] == 0 &&
				   commandParameters[3] == 0 &&
// TODO: must be called with 0 read offset or otherwise updated:
				   isCommandParameterString(data, commandParameters[1], NSIS_OUTPUT_DIRECTORY, versionInformation.type, unicode, stringsOffset, numberOfStringCharacters)) {
					versionInformation.isNSIS225 = true;
				}
			}
		}
	}

// TODO: is this needed?:
data.setReadOffset(initialReadOffset);

	if(!isStrongNSIS && !versionInformation.isNSIS200 && !versionInformation.isNSIS225) {
		std::optional<uint32_t> optionalCommandParameter;
		uint32_t commandParameters[NUMBER_OF_COMMAND_PARAMETERS];
		uint32_t commandMask = 0;

		for(uint32_t i = 0; i < numberOfEntries; i++) {
			// use original, unconverted command
			uint32_t commandID = data.readUnsignedInteger().value_or(magic_enum::enum_integer(Command::InvalidOperationCode));

			if(commandID == magic_enum::enum_integer(Command::InvalidOperationCode)) {
// TODO: err
spdlog::error("Failed to read command ID.");
				return {};
			}

			if(commandID < magic_enum::enum_integer(Command::WriteUninstaller) ||
			   commandID > magic_enum::enum_integer(Command::WriteUninstaller) + (unicode ? 4 : 2)) {
				continue;
			}

			for(uint8_t j = 0; j < NUMBER_OF_COMMAND_PARAMETERS; j++) {
				optionalCommandParameter = data.readUnsignedInteger();

				if(!optionalCommandParameter.has_value()) {
					spdlog::error("Failed to read command parameter #{} for entry #{}.", j + 1, i + 1);

					return {};
				}

				commandParameters[j] = optionalCommandParameter.value();
			}

			if(commandParameters[4] != 0 ||
			   commandParameters[5] != 0 ||
			   commandParameters[0] <= 1 ||
			   commandParameters[3] <= 1) {
				continue;
			}

			uint32_t alternateCommandParameter = commandParameters[3];
			uint32_t additional = std::numeric_limits<uint32_t>::max();

// TODO: must be called with 0 read offset (or updated otherwise):
			if(GetVarIndexFinished(data, alternateCommandParameter, '\\', versionInformation.type, unicode, stringsOffset, numberOfStringCharacters, &additional) != NSIS_INSTALL_DIRECTORY) {
				continue;
			}

// TODO: must be called with 0 read offset:
			if(AreTwoParamStringsEqual(data, alternateCommandParameter + additional, commandParameters[0], unicode, stringsOffset, numberOfStringCharacters)) {
				commandMask |= (1 << commandID - magic_enum::enum_integer(Command::WriteUninstaller));
			}
		}

		if(commandMask == 1) {
			// can be original NSIS or Park1
			wasParkVersionDetected = true;
		}
		else if(commandMask != 0) {
			Type newType = versionInformation.type;

			if(unicode) {
				switch(commandMask) {
					case 1 << 3: {
						newType = Type::Park2;
						break;
					}

					case 1 << 4: {
						newType = Type::Park3;
						break;
					}
				}
			}
			else {
				switch(commandMask) {
					case 1 << 1: {
						newType = Type::Park2;
						break;
					}

					case 1 << 2: {
						newType = Type::Park3;
						break;
					}
				}
			}

			if(newType != versionInformation.type) {
				wasParkVersionDetected = true;

				versionInformation.type = newType;
			}
		}
	}

// TODO: is this needed?:
data.setReadOffset(initialReadOffset);

	std::optional<Command> optionalBadCommand(getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled));

	if(optionalBadCommand < Command::RegisterDynamicLinkLibrary) {
		return versionInformation;
	}

	if(isStrongPark && !wasParkVersionDetected && !optionalBadCommand.has_value() || optionalBadCommand < Command::SectionSet) {
		versionInformation.type = Type::Park3;
		logCommandEnabled = true;
		optionalBadCommand = getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled);

		if(optionalBadCommand > Command::InvalidOperationCode && optionalBadCommand < Command::SectionSet) {
			versionInformation.type = Type::Park2;
			logCommandEnabled = false;
			optionalBadCommand = getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled);

			if(optionalBadCommand > Command::InvalidOperationCode && optionalBadCommand < Command::SectionSet) {
				versionInformation.type = Type::Park1;
				optionalBadCommand = getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled);
			}
		}
	}

	if(optionalBadCommand >= Command::SectionSet) {
		logCommandEnabled = !logCommandEnabled;

		optionalBadCommand = getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled);

		if(optionalBadCommand >= Command::SectionSet && logCommandEnabled) {
			logCommandEnabled = false;

			optionalBadCommand = getBadCommand(data, numberOfEntries, versionInformation.type, unicode, logCommandEnabled);
		}
	}

	return {};
}

std::unique_ptr<NullsoftScriptableInstallSystemArchive> NullsoftScriptableInstallSystemArchive::readFrom(const std::string & filePath) {
	static constexpr size_t NSIS_STEP = 512;
	static constexpr size_t NSIS_SIGNATURE_OFFSET = 4;
	static constexpr size_t NSIS_FIRST_HEADER_SIZE = 4 * 7;

	std::unique_ptr<ByteBuffer> data(ByteBuffer::readFrom(filePath, Endianness::LittleEndian));

	if(data == nullptr) {
spdlog::error("failed to read installer");
		return nullptr;
	}

	size_t startOffset = 0;
	uint64_t pePosition = std::numeric_limits<uint64_t>::max();

	// verify signature and locate pe position
	while(true) {
		if(!data->canReadBytes(NSIS_SIGNATURE.size() + NSIS_SIGNATURE_OFFSET)) {
// TODO:
spdlog::error("data truncated or missing signature");
			return nullptr;
		}

		if(std::memcmp(data->getRawData() + data->getReadOffset() + NSIS_SIGNATURE_OFFSET, NSIS_SIGNATURE.data(), NSIS_SIGNATURE.size()) == 0) {
// TODO: err
spdlog::info("signature found at {}", data->getReadOffset());
			break;
		}

		if(!data->canReadBytes(NSIS_FIRST_HEADER_SIZE)) {
// TODO: err
spdlog::error("data truncated or missing first header");
			return nullptr;
		}

		if(isArcPe(*data, data->getReadOffset())) {
// TODO: err
spdlog::info("is arc pe check succeeded");
			pePosition = static_cast<uint64_t>(data->getReadOffset());
		}

		if(!data->skipReadBytes(NSIS_STEP)) {
// TODO: err
spdlog::error("invalid or truncated data");
			return nullptr;
		}

		if(pePosition == 0 && data->getReadOffset() > 1 << 20) {
// TODO: err
spdlog::error("exceeded max search position for pe position");
			return nullptr;
		}
	}

	startOffset = data->getReadOffset();
	uint32_t peSize = 0;

	if(pePosition != std::numeric_limits<uint64_t>::max()) {
		uint64_t peSize64 = data->getReadOffset() - pePosition;

		if(peSize < (1 << 20)) {
			peSize = static_cast<uint32_t>(peSize64);
			startOffset = pePosition;
		}
	}

	size_t dataStreamOffset = data->getReadOffset() + NSIS_FIRST_HEADER_SIZE;

	std::optional<uint32_t> optionalFirstHeaderFlags(data->readUnsignedInteger());

// TODO: temp:
spdlog::info("optionalFirstHeaderFlags: {}", optionalFirstHeaderFlags.value());

	if(!optionalFirstHeaderFlags.has_value() || (optionalFirstHeaderFlags.value() & ~0xF) != 0) {
// TODO: err:
spdlog::error("missing or invalid first header flags");
		return nullptr;
	}

// TODO: reset buffer position to header start?

	// read first header flags
	FirstHeader firstHeader;
	firstHeader.flags = static_cast<FirstHeader::Flag>(optionalFirstHeaderFlags.value());

// TODO: temp:
std::stringstream firstHeaderFlagsStringStream;
for(FirstHeader::Flag flag : magic_enum::enum_values<FirstHeader::Flag>())
{
	if(Any(firstHeader.flags & flag)) {
		firstHeaderFlagsStringStream << magic_enum::enum_name(flag) << " ";
	}
}
spdlog::info("firstHeaderFlagString: '{}'", firstHeaderFlagsStringStream.str());

	// skip over the signature info and 'NullsoftInst' signature bytes
	data->skipReadBytes(NSIS_SIGNATURE.size());

	// read first header size
	std::optional<uint32_t> optionalFirstHeaderSize(data->readUnsignedInteger());

// TODO: for some reason the header size is always 1953721929 bytes?
	if(!optionalFirstHeaderSize.has_value() || optionalFirstHeaderSize.value() == 0) {
// TODO: err:
spdlog::error("missing or invalid first header size: {} (total: {})", optionalFirstHeaderSize.value(), data->getSize());
		return nullptr;
	}

	firstHeader.headerSize = optionalFirstHeaderSize.value();

	// read first header arc size
	std::optional<uint32_t> optionalDataSize(data->readUnsignedInteger());

	if(!optionalDataSize.has_value() || optionalDataSize.value() > data->getSize()) {
// TODO: err:
spdlog::error("missing or invalid first header arc size");
		return nullptr;
	}

// TODO: this check is in 7-zip but doesn't seem to make any sense?
//	if(optionalDataSize.value() < NSIS_FIRST_HEADER_SIZE) {
//spdlog::warn("arc size less than start header size");
//		return nullptr;
//	}

	firstHeader.dataSize = optionalDataSize.value();

	if(peSize != 0) {
		std::optional<std::vector<uint8_t>> optionalExecutableStub(data->getBytes(peSize, pePosition));

		if(!optionalExecutableStub.has_value()) {
// TODO: err
spdlog::error("failed to read exe stub");
			return nullptr;
		}
	}

// TODO: temp:
spdlog::info("ok - readOffset: {} pePos: {} peSize: {} startOffset: {}", data->getReadOffset(), pePosition, peSize, startOffset);
spdlog::info("headerSize: {} dataSize: {} totalSize: {}", firstHeader.headerSize, firstHeader.dataSize, data->getSize());

	//data->skipReadBytes(NSIS_FIRST_HEADER_SIZE);

	bool headerCompressed = true;
	bool solid = true;
	bool filterFlag = false;
	uint32_t dictionarySize = 1;
	MethodType methodType = MethodType::Deflate;
	std::optional<uint32_t> optionalCompressedHeaderSize(data->getUnsignedInteger(data->getReadOffset()));

	if(!optionalCompressedHeaderSize.has_value()) {
// TODO: err
spdlog::error("failed to read compressed header size value");
		return nullptr;
	}

	uint32_t compressedHeaderSize = optionalCompressedHeaderSize.value();

	if(compressedHeaderSize == firstHeader.headerSize) {
		headerCompressed = false;
		solid = false;
		methodType = MethodType::Copy;
	}
	else if(isLZMA(*data, dictionarySize, filterFlag)) {
		methodType = MethodType::LZMA;
	}
	else if((*data)[data->getReadOffset() + 3] == 0x80) {
		solid = false;
		data->skipReadBytes(4);

		if(isLZMA(*data, dictionarySize, filterFlag)) {
			methodType = MethodType::LZMA;
		}
		else if(isBZip2(*data)) {
			methodType = MethodType::BZip2;
		}
		else {
			methodType = MethodType::Deflate;
		}
	}
	else if(isBZip2(*data)) {
		methodType = MethodType::BZip2;
	}
	else {
		methodType = MethodType::Deflate;
	}

spdlog::info("method: {} solid: {}", magic_enum::enum_name(methodType), solid ? "Y" : "N");

// lzmaCheckIndex: 35852 (readOffset)
// newLZMACheckIndex: 35880 (lzma delta: -12)
// sigOffset: 35840 (lzma delta: 28)
// readOffset: 35852 (lzma delta: 16)
// expected lzma identifier index: 35868
// brute force lzma identifier:
/*uint32_t tempDictionarySize = 0;
for(size_t i = 0; i < data->getSize(); i++) {
	if(Utilities::isLZMA(*data, tempDictionarySize, i)) {
		spdlog::info("lzma identifier found at {}", i);
	}
}*/
// #2
// sigOffset: 34816 (lzma delta: 32)
// readOffset: 34828 (lzma delta: 20)
// expected lzma identifier: 34848

	static constexpr uint32_t NSIS_COMPRESSED_MASK = 1 << 31;

// TODO: debugging:
spdlog::info("readOffsetB: {} dataStreamOffset: {} solid: {}", data->getReadOffset(), dataStreamOffset, solid ? "Y" : "N");

	data->setReadOffset(dataStreamOffset);

	if(!solid) {
		headerCompressed = (compressedHeaderSize & NSIS_COMPRESSED_MASK) != 0;
		compressedHeaderSize &= ~NSIS_COMPRESSED_MASK;
		data->skipReadBytes(4);
	}

spdlog::info("readOffsetC: {} headerCompressed: {} compressedHeaderSize: {}", data->getReadOffset(), headerCompressed, compressedHeaderSize);

	ByteBuffer decompressedData;

// TODO: implement decompressor:
	if(headerCompressed) {
		std::optional<ByteBuffer::CompressionMethod> optionalCompressionMethod(methodTypeToCompressionMethod(methodType));

		if(!optionalCompressionMethod.has_value()) {
// TODO: err
spdlog::error("invalid compression method");
			return nullptr;
		}

		decompressedData = data->decompressed(optionalCompressionMethod.value(), data->getReadOffset(), firstHeader.dataSize - NSIS_FIRST_HEADER_SIZE - (solid ? 0 : 4));

// TODO: brute force lzma decompression test:
/*
ByteBuffer decompressedData;
size_t offset = data->getReadOffset();
size_t iterations = 1;
size_t iteration = 0;
while(true) {
	size_t currentSize = 1;
	size_t maxSize = data->getSize() - data->getReadOffset();

spdlog::info("iteration: {} maxSize: {}", iteration, maxSize);

	while(true) {
		spdlog::info("currentSize {}", currentSize);

		decompressedData = data->decompressed(optionalCompressionMethod.value(), offset, currentSize);

		if(decompressedData.isNotEmpty()) {
spdlog::info("wut?");
			break;
		}

		if(currentSize >= maxSize) {
spdlog::info("max size exceeded");
			break;
		}

		currentSize++;
	}

	if(decompressedData.isNotEmpty()) {
		break;
	}

	iteration++;

	if(iteration > iterations) {
		break;
	}
}
//*/

		if(decompressedData.isEmpty()) {
// TODO: err:
spdlog::error("DECOMPRESSION FAILED!");
			return nullptr;
		}

// TODO: temp:
spdlog::info("DECOMPRESSED!!");

		if(solid) {
			std::optional<uint32_t> optionalHeaderSize(decompressedData.getUnsignedInteger(decompressedData.getReadOffset()));

			if(optionalHeaderSize != firstHeader.headerSize) {
// TODO: err:
spdlog::error("header size doesn't match expected value");
				return nullptr;
			}
		}
	}
	else {
// TODO: temp:
spdlog::info("uncompressed - headerSize: {} remaining: {}", firstHeader.headerSize, data->getSize() - data->getReadOffset() - firstHeader.headerSize);

		std::optional<std::vector<uint8_t>> uncompressedData(data->readBytes(firstHeader.headerSize));

		if(!uncompressedData.has_value()) {
// TODO: err:
spdlog::error("failed to read uncompressed data");
			return nullptr;
		}

		decompressedData = std::move(uncompressedData.value());
	}

// TODO: temp:
spdlog::info("got decompressed data! size: {}", decompressedData.getSize());

	bool is64Bit = false;

	if(decompressedData.getSize() >= 4 + (12 * 8)) {
		is64Bit = true;

		for(size_t i = 0; i < 8; i++) {
			if(decompressedData.getUnsignedInteger(decompressedData.getReadOffset() + (4 + 12) * (i + 4)) != 0) {
				is64Bit = false;
				break;
			}
		}
	}

	uint8_t bhoSize = is64Bit ? 12 : 8;

// TODO: debugging:
spdlog::info("64bit: {} bhoSize: {}", is64Bit ? "Y" : "N", bhoSize);

	if(decompressedData.getSize() < 4 + static_cast<size_t>(bhoSize * 8)) {
// TODO: err:
spdlog::error("Invalid decompressed data size.");
		return nullptr;
	}

	if(!decompressedData.skipReadBytes(4 + static_cast<size_t>(bhoSize * 2))) {
		spdlog::error("Data truncated, missing entries block header.");

		return nullptr;
	}

	bool unicode = false;
	bool logCommandEnabled = false;
	Type type = Type::NSIS2;
	Command command = Command::InvalidOperationCode;
	uint32_t numberOfStringCharacters = 0;
	BlockHeader entriesBlockHeader;
	BlockHeader stringsBlockHeader;
	BlockHeader languageTablesBlockHeader;

// TODO: debugging:
/*
original block offset: 20
[2022-10-13 16:50:11.880] [info] final success count: 11
[2022-10-13 16:50:11.880] [info] offset #1: 61
[2022-10-13 16:50:11.880] [info] offset #2: 257
[2022-10-13 16:50:11.880] [info] offset #3: 477
[2022-10-13 16:50:11.880] [info] offset #4: 2545
[2022-10-13 16:50:11.880] [info] offset #5: 2546
[2022-10-13 16:50:11.880] [info] offset #6: 2573
[2022-10-13 16:50:11.880] [info] offset #7: 2581
[2022-10-13 16:50:11.881] [info] offset #8: 2631
[2022-10-13 16:50:11.881] [info] offset #9: 4381
[2022-10-13 16:50:11.881] [info] offset #10: 4433
[2022-10-13 16:50:11.881] [info] offset #11: 4453
*/
spdlog::info("original block offset: {}", decompressedData.getReadOffset());
//decompressedData.setReadOffset(4381); // TODO: test override offset
/*bool bruteForceTest = false;
size_t testOffset = 0;
std::vector<std::pair<size_t, Type>> successOffsets;
while(true) {
if(bruteForceTest) {
testOffset++;
spdlog::info("test offset: {}", testOffset - 1);
decompressedData.setReadOffset(testOffset - 1);
if(testOffset >= 32) {
break;
}
}*/

	if(!entriesBlockHeader.parse(decompressedData, bhoSize)) {
// TODO: err:
spdlog::error("Failed to parse entries block header.");
		return nullptr;
	}

	decompressedData.skipReadBytes(bhoSize);

	if(!stringsBlockHeader.parse(decompressedData, bhoSize)) {
// TODO: err:
spdlog::error("Failed to parse strings block header.");
		return nullptr;
	}

	decompressedData.skipReadBytes(bhoSize);

	if(!languageTablesBlockHeader.parse(decompressedData, bhoSize)) {
// TODO: err:
spdlog::error("Failed to parse language tables block header.");
		return nullptr;
	}

	decompressedData.skipReadBytes(bhoSize);

// TODO: debugging:
spdlog::info("block headers parsed - remaining: {}", decompressedData.getSize() - decompressedData.getReadOffset());

// TODO: brute force debugging:
/*if(bruteForceTest) {
if(decompressedData.getSize() - decompressedData.getReadOffset() == 0) {
spdlog::info("final success count: {}", successOffsets.size());
for(size_t i = 0; i < successOffsets.size(); i++) {
spdlog::info("offset #{}: {} ({})", i + 1, successOffsets[i].first, magic_enum::enum_name(successOffsets[i].second));
}
break;
}
}*/

	decompressedData.setReadOffset(entriesBlockHeader.offset);

	if(entriesBlockHeader.offset == 0 ||
	   stringsBlockHeader.offset == 0 ||
	   languageTablesBlockHeader.offset == 0 ||
	   entriesBlockHeader.offset > decompressedData.getSize() ||
	   stringsBlockHeader.offset > decompressedData.getSize() ||
	   languageTablesBlockHeader.offset > decompressedData.getSize()) {
// TODO: err
spdlog::error("invalid block header offset - decompressedDataSize: {}", decompressedData.getSize());
spdlog::info("stringsOffset: {} languageTablesOffset: {} entriesOffset: {}",  stringsBlockHeader.offset, languageTablesBlockHeader.offset, entriesBlockHeader.offset);
spdlog::info("stringsNum: {} languageTablesNum: {} entriesNum: {}", stringsBlockHeader.num, languageTablesBlockHeader.num, entriesBlockHeader.num);
//if(bruteForceTest) continue;
		return nullptr;
	}

	if(languageTablesBlockHeader.offset < stringsBlockHeader.offset) {
// TODO: err
spdlog::error("invalid language tables or strings block header offset");
//if(bruteForceTest) continue;
		return nullptr;
	}

	uint32_t stringTableSize = languageTablesBlockHeader.offset - stringsBlockHeader.offset;

	if(stringTableSize < 2) {
// TODO: err
spdlog::error("invalid string table size");
//if(bruteForceTest) continue;
		return nullptr;
	}

	if(decompressedData.getUnsignedByte(decompressedData.getReadOffset() + stringsBlockHeader.offset + stringTableSize - 1) != 0) {
// TODO: err
spdlog::error("some random undocumented check failed");
//if(bruteForceTest) continue;
		return nullptr;
	}

	unicode = decompressedData.getUnsignedInteger(decompressedData.getReadOffset() + stringsBlockHeader.offset) == 0;
	numberOfStringCharacters = stringTableSize;

	if(unicode) {
		if((stringTableSize & 1) != 0) {
// TODO: err
spdlog::error("string table size check failed");
//if(bruteForceTest) continue;
			return nullptr;
		}

		numberOfStringCharacters >>= 1;

		if(decompressedData.getUnsignedByte(decompressedData.getReadOffset() + stringsBlockHeader.offset + stringTableSize - 2) != 0) {
// TODO: err
spdlog::error("other random undocumented check failed");
//if(bruteForceTest) continue;
			return nullptr;
		}
	}

	if(entriesBlockHeader.num > (1 << 25) ||
	   entriesBlockHeader.num * static_cast<size_t>(COMMAND_SIZE) > decompressedData.getSize() - entriesBlockHeader.offset) {
// TODO: err
spdlog::error("invalid entries block header count");
//if(bruteForceTest) continue;
		return nullptr;
	}

	VersionInformation versionInformation = detectVersion(decompressedData, unicode, logCommandEnabled, entriesBlockHeader.num, entriesBlockHeader.offset, stringsBlockHeader.offset, numberOfStringCharacters);

// TODO: debugging:
spdlog::info("version: {} 200: {} 223: {}", magic_enum::enum_name(versionInformation.type), versionInformation.isNSIS200 ? "Y" : "N", versionInformation.isNSIS225 ? "Y" : "N");
spdlog::info("       entries offset: {} size: {}", entriesBlockHeader.offset, entriesBlockHeader.num);
spdlog::info("       strings offset: {} size: {}", stringsBlockHeader.offset, stringsBlockHeader.num);
spdlog::info("languageTables offset: {} size: {}", languageTablesBlockHeader.offset, languageTablesBlockHeader.num);

/*
if(bruteForceTest) {
successOffsets.emplace_back(std::make_pair(testOffset - 1, versionInformation.type));
spdlog::warn("SUCCESS! @ {} (count: {})", testOffset - 1, successOffsets.size());
//break;
}

if(!bruteForceTest) {
break;
}
}*/

	std::vector<std::string> unicodePrefixes = { std::string("$INSTDIR") };
	std::vector<std::string> asciiPrefixes = { std::string("$INSTDIR") };

	const uint32_t specialOutputDirectoryVariableIndex = versionInformation.isNSIS225 ? NSIS_225_SPECIAL_OUTPUT_DIRECTORY : NSIS_226_SPECIAL_OUTPUT_DIRECTORY;
	uint32_t commandParameters[NUMBER_OF_COMMAND_PARAMETERS];

	for(size_t i = 0; i < entriesBlockHeader.num; i++) {
		decompressedData.setReadOffset(entriesBlockHeader.offset + (i * COMMAND_SIZE));

		std::optional<uint32_t> optionalCommandID = decompressedData.readUnsignedInteger();

		if(!optionalCommandID.has_value()) {
			spdlog::error("Failed to read command ID #{} from entries block.", i + 1);
		}

		command = getConvertedCommand(optionalCommandID.value(), type, unicode, logCommandEnabled);

		for(size_t j = 0; j < NUMBER_OF_COMMAND_PARAMETERS; j++) {
			std::optional<uint32_t> optionalCommandParameter(decompressedData.readUnsignedInteger());

			if(!optionalCommandParameter.has_value()) {
				return nullptr;
			}

			commandParameters[j] = optionalCommandParameter.value();
		}

		switch(command) {
			case Command::CreateDirectory: {
				bool isSetOutputPath = commandParameters[1] != 0;

				if(!isSetOutputPath) {
					break;
				}

				uint32_t parameter0 = commandParameters[0];
				uint32_t offset = std::numeric_limits<uint32_t>::max();

				int32_t variableIndex = GetVarIndex(decompressedData, parameter0, versionInformation.type, unicode, stringsBlockHeader.offset, numberOfStringCharacters, &offset);

				if(variableIndex == specialOutputDirectoryVariableIndex ||
				   variableIndex == NSIS_OUTPUT_DIRECTORY) {
					parameter0 += offset;
				}

// TODO:
				//ReadString2_Raw(par0);

				if(unicode) {
					if(variableIndex == specialOutputDirectoryVariableIndex) {
						//Raw_UString.Insert(0, spec_outdir_U);
					}
					else if(variableIndex == NSIS_OUTPUT_DIRECTORY) {
						//Raw_UString.Insert(0, UPrefixes.Back());
					}

					//UPrefixes.Add(Raw_UString);
				}
				else {
					if(variableIndex == specialOutputDirectoryVariableIndex) {
						//Raw_AString.Insert(0, spec_outdir_A);
					}
					else if(variableIndex == NSIS_OUTPUT_DIRECTORY) {
						//Raw_AString.Insert(0, APrefixes.Back());
					}

					//APrefixes.Add(Raw_AString);
				}

				break;
			}

			case Command::AssignVariable: {
				if(commandParameters[0] != specialOutputDirectoryVariableIndex) {
					break;
				}

// TODO:
				//spec_outdir_U.Empty();
				//spec_outdir_A.Empty();

				if(isCommandParameterString(decompressedData, commandParameters[1], NSIS_OUTPUT_DIRECTORY, versionInformation.type, unicode, stringsBlockHeader.offset, numberOfStringCharacters) &&
				   commandParameters[2] == 0 &&
				   commandParameters[3] == 0) {
					//spec_outdir_U = UPrefixes.Back(); // outdir_U;
					//spec_outdir_A = APrefixes.Back(); // outdir_A;
				}

				break;
			}

			case Command::ExtractFile: {
// TODO
				break;
			}

			case Command::SetFileAttributes: {
// TODO
				break;
			}

			case Command::WriteUninstaller: {
// TODO
				break;
			}
		}

// TODO: finish me
	}

	spdlog::info("end");

// TODO:
	return std::unique_ptr<NullsoftScriptableInstallSystemArchive>(new NullsoftScriptableInstallSystemArchive(filePath));
}

const std::vector<std::shared_ptr<NullsoftScriptableInstallSystemArchive::Entry>> & NullsoftScriptableInstallSystemArchive::getEntries() const {
	return m_entries;
}

std::vector<std::shared_ptr<NullsoftScriptableInstallSystemArchive::Entry>> & NullsoftScriptableInstallSystemArchive::getEntries() {
	return m_entries;
}
