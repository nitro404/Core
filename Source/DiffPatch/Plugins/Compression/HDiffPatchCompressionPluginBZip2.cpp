#include "HDiffPatchCompressionPluginBZip2.h"

#include <bzlib.h>

/*
#ifdef  _CompressPlugin_bz2
#if (_IsNeedIncludeDefaultCompressHead)
#   include "bzlib.h" // http://www.bzip.org/  https://github.com/sisong/bzip2
#endif
    typedef struct{
        hdiff_TCompress base;
        int             compress_level; //0..9
    } TCompressPlugin_bz2;
    static hpatch_StreamPos_t _bz2_compress(const hdiff_TCompress* compressPlugin,
                                            const hpatch_TStreamOutput* out_code,
                                            const hpatch_TStreamInput*  in_data){
        const TCompressPlugin_bz2* plugin=(const TCompressPlugin_bz2*)compressPlugin;
        hpatch_StreamPos_t result=0;
        const char*        errAt="";
        unsigned char*     _temp_buf=0;
        bz_stream          s;
        unsigned char*     code_buf,* data_buf;
        int                is_eof=0;
        int                is_stream_end=0;
        hpatch_StreamPos_t readFromPos=0;
        int                outStream_isCanceled=0;
        memset(&s,0,sizeof(s));
        
        _temp_buf=(unsigned char*)malloc(kCompressBufSize*2);
        if (!_temp_buf) _compress_error_return("memory alloc");
        code_buf=_temp_buf;
        data_buf=_temp_buf+kCompressBufSize;
        s.next_out = (char*)code_buf;
        s.avail_out = kCompressBufSize;
        if (BZ_OK!=BZ2_bzCompressInit(&s,plugin->compress_level, 0, 0))
            _compress_error_return("BZ2_bzCompressInit()");
        while (1) {
            if ((s.avail_out<kCompressBufSize)|is_stream_end){
                size_t writeLen=kCompressBufSize-s.avail_out;
                if (writeLen>0){
                    _stream_out_code_write(out_code,outStream_isCanceled,result,code_buf,writeLen);
                }
                s.next_out=(char*)code_buf;
                s.avail_out=kCompressBufSize;
                if (is_stream_end)
                    break;//end loop
            }else{
                if (s.avail_in>0){
                    if (BZ_RUN_OK!=BZ2_bzCompress(&s,BZ_RUN)) _compress_error_return("BZ2_bzCompress()");
                }else if (is_eof){
                    int ret=BZ2_bzCompress(&s,BZ_FINISH);
                    is_stream_end= (ret==BZ_STREAM_END);
                    if ((ret!=BZ_STREAM_END)&&(ret!=BZ_FINISH_OK))
                        _compress_error_return("BZ2_bzCompress() BZ_FINISH");
                }else{
                    size_t readLen=kCompressBufSize;
                    if (readFromPos+readLen>in_data->streamSize)
                        readLen=(size_t)(in_data->streamSize-readFromPos);
                    if (readLen==0){
                        is_eof=1;
                    }else{
                        if (!in_data->read(in_data,readFromPos,data_buf,data_buf+readLen))
                            _compress_error_return("in_data->read()");
                        readFromPos+=readLen;
                        s.next_in=(char*)data_buf;
                        s.avail_in=(unsigned int)readLen;
                    }
                }
            }
        }
    clear:
        if (BZ_OK!=BZ2_bzCompressEnd(&s))
            { result=kCompressFailResult; if (strlen(errAt)==0) errAt="BZ2_bzCompressEnd()"; }
        _check_compress_result(result,outStream_isCanceled,"_bz2_compress()",errAt);
        if (_temp_buf) free(_temp_buf);
        return result;
    }
    _def_fun_compressType(_bz2_compressType,"bz2");
    static const TCompressPlugin_bz2 bz2CompressPlugin={
        {_bz2_compressType,_default_maxCompressedSize,_default_setParallelThreadNumber,_bz2_compress}, 9};
    
#   if (_IS_USED_MULTITHREAD)
    //pbz2
    typedef struct{
        TCompressPlugin_bz2     base;
        int                     thread_num; // 1..
        hdiff_TParallelCompress pc;
    } TCompressPlugin_pbz2;
    static int _pbz2_setThreadNum(hdiff_TCompress* compressPlugin,int threadNum){
        TCompressPlugin_pbz2* plugin=(TCompressPlugin_pbz2*)compressPlugin;
        plugin->thread_num=threadNum;
        return threadNum;
    }
    static hdiff_compressBlockHandle _pbz2_openBlockCompressor(hdiff_TParallelCompress* pc){
        return pc;
    }
    static void _pbz2_closeBlockCompressor(hdiff_TParallelCompress* pc,
                                           hdiff_compressBlockHandle blockCompressor){
        assert(blockCompressor==pc);
    }
    static
    size_t _pbz2_compressBlock(hdiff_TParallelCompress* pc,hdiff_compressBlockHandle blockCompressor,
                               hpatch_StreamPos_t blockIndex,hpatch_StreamPos_t blockCount,unsigned char* out_code,unsigned char* out_codeEnd,
                               const unsigned char* block_data,const unsigned char* block_dictEnd,const unsigned char* block_dataEnd){
        const TCompressPlugin_pbz2* plugin=(const TCompressPlugin_pbz2*)pc->import;
        unsigned int codeLen=(unsigned int)(out_codeEnd-out_code);
        if (BZ_OK!=BZ2_bzBuffToBuffCompress((char*)out_code,&codeLen,(char*)block_data,
                                            (unsigned int)(block_dataEnd-block_data),
                                            plugin->base.compress_level,0,0)) return 0; //error
        return codeLen;
    }
    static hpatch_StreamPos_t _pbz2_compress(const hdiff_TCompress* compressPlugin,
                                             const hpatch_TStreamOutput* out_code,
                                             const hpatch_TStreamInput*  in_data){
        TCompressPlugin_pbz2* plugin=(TCompressPlugin_pbz2*)compressPlugin;
        const size_t blockSize=plugin->base.compress_level*100000;
        if ((plugin->thread_num<=1)||(plugin->base.compress_level==0)
                ||(in_data->streamSize<blockSize*2)){ //same as "bz2"
            return _bz2_compress(compressPlugin,out_code,in_data);
        }else{
            plugin->pc.import=plugin;
            return parallel_compress_blocks(&plugin->pc,plugin->thread_num,0,blockSize,out_code,in_data);
        }
    }
    
    _def_fun_compressType(_pbz2_compressType,"pbz2");
    static const TCompressPlugin_pbz2 pbz2CompressPlugin={
        { {_pbz2_compressType,_default_maxCompressedSize,_pbz2_setThreadNum,_pbz2_compress}, 8},
        kDefaultCompressThreadNumber ,{0,_default_maxCompressedSize,_pbz2_openBlockCompressor,
            _pbz2_closeBlockCompressor,_pbz2_compressBlock} };
#   endif // _IS_USED_MULTITHREAD
#endif//_CompressPlugin_bz2
*/
