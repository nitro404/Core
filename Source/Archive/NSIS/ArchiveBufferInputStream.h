#ifndef _ARCHIVE_BUFFER_INPUT_STREAM_H_
#define _ARCHIVE_BUFFER_INPUT_STREAM_H_

#include "ByteBuffer.h"

#include <SevenZip/CPP/7zip/IStream.h>
#include <SevenZip/CPP/Common/MyCom.h>

class ArchiveBufferInputStream final : public IInStream,
									   public CMyUnknownImp {
public:
	ArchiveBufferInputStream(std::unique_ptr<ByteBuffer> data);
	virtual ~ArchiveBufferInputStream();

	MY_UNKNOWN_IMP1(IInStream);

	// IInStream Virtuals
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
	STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);

private:
	std::unique_ptr<ByteBuffer> m_data;
};

#endif // _ARCHIVE_BUFFER_INPUT_STREAM_H_
