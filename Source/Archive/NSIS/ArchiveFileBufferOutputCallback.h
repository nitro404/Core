#ifndef _ARCHIVE_FILE_BUFFER_OUTPUT_CALLBACK_H_
#define _ARCHIVE_FILE_BUFFER_OUTPUT_CALLBACK_H_

#include "ByteBuffer.h"
#include "NullsoftScriptableInstallSystemArchive.h"

#include <SevenZip/CPP/7zip/IPassword.h>
#include <SevenZip/CPP/Common/MyCom.h>
#include <SevenZip/CPP/Common/MyString.h>

#include <memory>
#include <optional>
#include <string>

class ArchiveFileBufferOutputStream;

class ArchiveFileBufferOutputCallback final : public IArchiveExtractCallback,
											  public ICryptoGetTextPassword,
											  public CMyUnknownImp {
public:
	ArchiveFileBufferOutputCallback(const NullsoftScriptableInstallSystemArchive::Entry & entry);
	virtual ~ArchiveFileBufferOutputCallback();

	void setPassword(const std::string & password);
	void clearPassword();

	ByteBuffer * getData();
	const ByteBuffer * getData() const;
	std::unique_ptr<ByteBuffer> transferData();

	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

	// IProgress Virtuals
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 * completeValue);

	// IArchiveExtractCallback Virtuals
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode);
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

	// ICryptoGetTextPassword Virtuals
	STDMETHOD(CryptoGetTextPassword)(BSTR * password);

private:
	const NullsoftScriptableInstallSystemArchive::Entry & m_entry;
	ArchiveFileBufferOutputStream * m_bufferOutputStreamSpec;
	CMyComPtr<IOutStream> m_bufferOutputStream;
	std::optional<UString> m_password;
	bool m_extractMode;
	std::unique_ptr<ByteBuffer> m_data;
};

#endif // _ARCHIVE_FILE_BUFFER_OUTPUT_CALLBACK_H_
