#ifndef _ARCHIVE_EXTRACT_FILE_CALLBACK_H_
#define _ARCHIVE_EXTRACT_FILE_CALLBACK_H_

#include "NullsoftScriptableInstallSystemArchive.h"

#include <SevenZip/CPP/7zip/IPassword.h>
#include <SevenZip/CPP/7zip/Common/FileStreams.h>
#include <SevenZip/CPP/Common/MyCom.h>

#include <optional>
#include <string>

class ArchiveExtractFileCallback final : public IArchiveExtractCallback,
										 public ICryptoGetTextPassword,
										 public CMyUnknownImp {
public:
	ArchiveExtractFileCallback(NullsoftScriptableInstallSystemArchive::Entry & entry, const std::string & outputFilePath, bool overwrite);
	virtual ~ArchiveExtractFileCallback();

	void setPassword(const std::string & password);
	void clearPassword();

	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

	// IProgress Virtuals
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 * completeValue);

	// IArchiveExtractFileCallback Virtuals
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode);
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

	// ICryptoGetTextPassword Virtuals
	STDMETHOD(CryptoGetTextPassword)(BSTR * password);

private:
	NullsoftScriptableInstallSystemArchive::Entry & m_entry;
	COutFileStream * m_outFileStreamSpec;
	CMyComPtr<ISequentialOutStream> m_outFileStream;
	FString m_outputFilePath;
	bool m_overwrite;
	FString m_diskFilePath;
	bool m_extractMode;
	std::optional<UString> m_password;
	std::optional<FILETIME> m_fileModifiedTimestamp;
	std::optional<UInt32> m_fileAttributes;
};

#endif // _ARCHIVE_EXTRACT_FILE_CALLBACK_H_
