#ifndef _ARCHIVE_OPEN_CALLBACK_H_
#define _ARCHIVE_OPEN_CALLBACK_H_

#include <SevenZip/CPP/7zip/IPassword.h>
#include <SevenZip/CPP/7zip/Archive/IArchive.h>
#include <SevenZip/CPP/Common/MyCom.h>
#include <SevenZip/CPP/Common/MyString.h>

#include <optional>
#include <string>

class ArchiveOpenCallback final : public IArchiveOpenCallback,
								  public ICryptoGetTextPassword,
								  public CMyUnknownImp {
public:
	ArchiveOpenCallback();
	virtual ~ArchiveOpenCallback();

	void setPassword(const std::string & password);
	void clearPassword();

	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

	// IArchiveOpenCallback Virtuals
	STDMETHOD(SetTotal)(const UInt64 * files, const UInt64 * bytes);
	STDMETHOD(SetCompleted)(const UInt64 * files, const UInt64 * bytes);

	// ICryptoGetTextPassword Virtuals
	STDMETHOD(CryptoGetTextPassword)(BSTR * password);

private:
	std::optional<UString> m_password;
};

#endif // _ARCHIVE_OPEN_CALLBACK_H_
