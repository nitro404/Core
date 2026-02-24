#include "HDiffPatchCompressionPluginZStandard.h"

#include <zstd.h>

/*
#ifdef  _CompressPlugin_zstd
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zstd.h" // "zstd/lib/zstd.h" https://github.com/sisong/zstd
#endif
	struct TCompressPlugin_zstd{
		hdiff_TCompress base;
		int             compress_level; //0..22
		int             dict_bits;  // 10..(30 or 31)
		int             thread_num;     //1..(200?)
	};
	static int _zstd_setThreadNumber(hdiff_TCompress* compressPlugin,int threadNum){
		TCompressPlugin_zstd* plugin=(TCompressPlugin_zstd*)compressPlugin;
		#define ZSTDMT_NBWORKERS_MAX 200
		if (threadNum>ZSTDMT_NBWORKERS_MAX) threadNum=ZSTDMT_NBWORKERS_MAX;
		plugin->thread_num=threadNum;
		return threadNum;
	}
	static hpatch_StreamPos_t _zstd_compress(const hdiff_TCompress* compressPlugin,
											 const hpatch_TStreamOutput* out_code,
											 const hpatch_TStreamInput*  in_data){
		const TCompressPlugin_zstd* plugin=(const TCompressPlugin_zstd*)compressPlugin;
		hpatch_StreamPos_t  result=0;
		const char*         errAt="";
		unsigned char*      _temp_buf=0;
		ZSTD_inBuffer       s_input;
		ZSTD_outBuffer      s_output;
#if (IS_REUSE_compress_handle)
		static ZSTD_CCtx*   s=0;
#else
		ZSTD_CCtx*          s=0;
#endif
		hpatch_StreamPos_t  readFromPos=0;
		int                 outStream_isCanceled=0;
		int                 dict_bits;
		size_t              ret;

		s_input.size=ZSTD_CStreamInSize();
		s_output.size=ZSTD_CStreamOutSize();
		_temp_buf=(unsigned char*)malloc(s_input.size+s_output.size);
		if (!_temp_buf) _compress_error_return("memory alloc");
		s_input.src=_temp_buf;
		s_output.dst=_temp_buf+s_input.size;
		
		if (!s) s=ZSTD_createCCtx();
		else ZSTD_CCtx_reset(s,ZSTD_reset_session_only);
		if (!s) _compress_error_return("ZSTD_createCCtx()");
		ret=ZSTD_CCtx_setParameter(s,ZSTD_c_compressionLevel,plugin->compress_level);
		if (ZSTD_isError(ret)) _compress_error_return("ZSTD_CCtx_setParameter(,ZSTD_c_compressionLevel)");
		ZSTD_CCtx_setPledgedSrcSize(s,in_data->streamSize);
		#define _ZSTD_WINDOWLOG_MIN 10
		dict_bits=plugin->dict_bits;
		while (((((hpatch_StreamPos_t)1)<<(dict_bits-1)) >= in_data->streamSize)
				&&((dict_bits-1)>=_ZSTD_WINDOWLOG_MIN)) {
			--dict_bits;
		}
#       if (IS_NOTICE_compress_canceled)
		printf("    (used one zstd dictSize: %" PRIu64 "  (input data: %" PRIu64 "))\n",
			   ((hpatch_StreamPos_t)1)<<dict_bits,in_data->streamSize);
#       endif
		ret=ZSTD_CCtx_setParameter(s,ZSTD_c_windowLog,dict_bits);
		if (ZSTD_isError(ret)) _compress_error_return("ZSTD_CCtx_setParameter(,ZSTD_c_windowLog)");
		if (plugin->thread_num>1){
			ret=ZSTD_CCtx_setParameter(s, ZSTD_c_nbWorkers,plugin->thread_num);
			//if (ZSTD_isError(ret)) printf("  (NOTICE: zstd unsupport multi-threading, warning.)\n");
		}

		for (;;){
			if (readFromPos<in_data->streamSize){
				s_input.pos=0;
				if (s_input.size>(in_data->streamSize-readFromPos))
					s_input.size=(size_t)(in_data->streamSize-readFromPos);
				if (!in_data->read(in_data,readFromPos,(unsigned char*)s_input.src,
								(unsigned char*)s_input.src+s_input.size))
					_compress_error_return("in_data->read()");
				readFromPos+=s_input.size;
			}
			{
				const int lastChunk=(readFromPos==in_data->streamSize);
				const ZSTD_EndDirective mode=lastChunk?ZSTD_e_end:ZSTD_e_continue;
				int finished;
				do {
					s_output.pos=0;
					ret=ZSTD_compressStream2(s,&s_output,&s_input,mode);
					if (ZSTD_isError(ret)) _compress_error_return("ZSTD_compressStream2()");
					if (s_output.pos>0){
						_stream_out_code_write(out_code,outStream_isCanceled,result,
											(const unsigned char*)s_output.dst,s_output.pos);
					}
					finished=lastChunk?(ret==0):(s_input.pos==s_input.size);
				} while (!finished);
				//if (s_input.pos!=s_input.size) _compress_error_return("Impossible: zstd only returns 0 when the input is completely consumed!");
				if (lastChunk)
					break;
			}
		}
	clear:
#if (!IS_REUSE_compress_handle)
		if (0!=ZSTD_freeCCtx(s))
		{ s=0; result=kCompressFailResult; if (strlen(errAt)==0) errAt="ZSTD_freeCStream()"; }
#endif
		_check_compress_result(result,outStream_isCanceled,"_zstd_compress()",errAt);
		if (_temp_buf) free(_temp_buf);
		return result;
	}
	_def_fun_compressType(_zstd_compressType,"zstd");
	static TCompressPlugin_zstd zstdCompressPlugin={
		{_zstd_compressType,_default_maxCompressedSize,_zstd_setThreadNumber,_zstd_compress},
		20,24,kDefaultCompressThreadNumber};
#endif//_CompressPlugin_zstd
*/
