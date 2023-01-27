#ifndef _ARCHIVE_BUFFER_OUTPUT_STREAM_H_
#define _ARCHIVE_BUFFER_OUTPUT_STREAM_H_

#include "ByteBuffer.h"

#include <SevenZip/CPP/7zip/IStream.h>
#include <SevenZip/CPP/Common/MyCom.h>

class ArchiveFileBufferOutputStream final : public IOutStream,
											public CMyUnknownImp {
public:
	ArchiveFileBufferOutputStream(ByteBuffer * data);
	virtual ~ArchiveFileBufferOutputStream();

	ByteBuffer * getData();
	const ByteBuffer * getData() const;

	MY_UNKNOWN_IMP1(IOutStream);

	// IOutStream Virtuals
	STDMETHOD(Write)(const void * data, UInt32 size, UInt32 * processedSize);
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition);
	STDMETHOD(SetSize)(UInt64 newSize);

private:
	ByteBuffer * m_data;
};

#endif // _ARCHIVE_BUFFER_OUTPUT_STREAM_H_
