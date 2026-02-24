#include "HDiffPatchCompressionPluginZLib.h"

#include <zlib.h>

/*
#ifdef  _CompressPlugin_zlib
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zlib.h" // http://zlib.net/  https://github.com/madler/zlib
#endif
    typedef struct{
        hdiff_TCompress base;
        int             compress_level; //0..9
        int             mem_level;
        signed char     windowBits; // -9..-15
        hpatch_BOOL     isNeedSaveWindowBits;
        int             strategy;
    } TCompressPlugin_zlib;
    typedef struct _zlib_TCompress{
        const hpatch_TStreamOutput* out_code;
        unsigned char*  c_buf;
        size_t          c_buf_size;
        z_stream        c_stream;
    } _zlib_TCompress;

    static _zlib_TCompress*  _zlib_compress_open_at(const hdiff_TCompress* compressPlugin,
                                                    int compressLevel,int compressMemLevel,
                                                    const hpatch_TStreamOutput* out_code,
                                                    _zlib_TCompress* self,size_t _self_and_buf_size){
        const TCompressPlugin_zlib* plugin=(const TCompressPlugin_zlib*)compressPlugin;
        assert(_self_and_buf_size>sizeof(_zlib_TCompress));
        memset(self,0,sizeof(_zlib_TCompress));
        self->c_buf=((unsigned char*)self)+sizeof(_zlib_TCompress);;
        self->c_buf_size=_self_and_buf_size-sizeof(_zlib_TCompress);
        self->out_code=out_code;
        
        self->c_stream.next_out = (Bytef*)self->c_buf;
        self->c_stream.avail_out = (uInt)self->c_buf_size;
        if (Z_OK!=deflateInit2(&self->c_stream,compressLevel,Z_DEFLATED,
                               plugin->windowBits,compressMemLevel,plugin->strategy))
            return 0;
        return self;
    }
    static _zlib_TCompress*  _zlib_compress_open_by(const hdiff_TCompress* compressPlugin,
                                                    int compressLevel,int compressMemLevel,
                                                    const hpatch_TStreamOutput* out_code,
                                                    unsigned char* _mem_buf,size_t _mem_buf_size){
        #define __MAX_TS(a,b)  ((a)>=(b)?(a):(b))
        const hpatch_size_t kZlibAlign=__MAX_TS(__MAX_TS(sizeof(hpatch_StreamPos_t),sizeof(void*)),sizeof(uLongf));
        #undef __MAX_TS
        unsigned char* _mem_buf_end=_mem_buf+_mem_buf_size;
        unsigned char* self_at=(unsigned char*)_hpatch_align_upper(_mem_buf,kZlibAlign);
        if (self_at>=_mem_buf_end) return 0;
        return _zlib_compress_open_at(compressPlugin,compressLevel,compressMemLevel,out_code,
                                      (_zlib_TCompress*)self_at,_mem_buf_end-self_at);
    }
    static hpatch_BOOL _zlib_compress_close_by(const hdiff_TCompress* compressPlugin,_zlib_TCompress* self){
        hpatch_BOOL result=hpatch_TRUE;
        if (!self) return result;
        if (self->c_stream.state!=0){
            int ret=deflateEnd(&self->c_stream);
            result=(Z_OK==ret)|(Z_DATA_ERROR==ret);
        }
        memset(self,0,sizeof(_zlib_TCompress));
        return result;
    }
    static int _zlib_compress_part(_zlib_TCompress* self,
                                   const unsigned char* part_data,const unsigned char* part_data_end,
                                   int is_data_end,hpatch_StreamPos_t* curWritedPos,int* outStream_isCanceled){
        int                 result=1; //true
        const char*         errAt="";
        int                 is_stream_end=0;
        int                 is_eof=0;
        assert(part_data<=part_data_end);
        self->c_stream.next_in=(Bytef*)part_data;
        self->c_stream.avail_in=(uInt)(part_data_end-part_data);
        while (1) {
            if ((self->c_stream.avail_out<self->c_buf_size)|is_stream_end){
                size_t writeLen=self->c_buf_size-self->c_stream.avail_out;
                if (writeLen>0){
                    _stream_out_code_write(self->out_code,*outStream_isCanceled,
                                           (*curWritedPos),self->c_buf,writeLen);
                }
                self->c_stream.next_out=(Bytef*)self->c_buf;
                self->c_stream.avail_out=(uInt)self->c_buf_size;
                if (is_stream_end)
                    break;//end loop
            }else{
                if (self->c_stream.avail_in>0){
                    if (Z_OK!=deflate(&self->c_stream,Z_NO_FLUSH)) _compress_error_return("deflate()");
                }else if (is_eof){
                    int ret=deflate(&self->c_stream,Z_FINISH);
                    is_stream_end= (ret==Z_STREAM_END);
                    if ((ret!=Z_STREAM_END)&&(ret!=Z_OK))
                        _compress_error_return("deflate() Z_FINISH");
                }else{
                    if (!is_data_end)
                        break;//part ok
                    else
                        is_eof=1;
                }
            }
        }
    clear:
        _check_compress_result(result,*outStream_isCanceled,"_zlib_compress_part()",errAt);
        return result;
    }

    static hpatch_StreamPos_t _zlib_compress(const hdiff_TCompress* compressPlugin,
                                             const hpatch_TStreamOutput* out_code,
                                             const hpatch_TStreamInput*  in_data){
        const TCompressPlugin_zlib* plugin=(const TCompressPlugin_zlib*)compressPlugin;
        hpatch_StreamPos_t result=0; //writedPos
        hpatch_StreamPos_t readFromPos=0;
        const char*        errAt="";
        int                 outStream_isCanceled=0;
        unsigned char*     _temp_buf=0;
        unsigned char*     data_buf=0;
        _zlib_TCompress*   self=0;
        _temp_buf=(unsigned char*)malloc(sizeof(_zlib_TCompress)+kCompressBufSize*2);
        if (!_temp_buf) _compress_error_return("memory alloc");
        self=_zlib_compress_open_by(compressPlugin,plugin->compress_level,plugin->mem_level,
                                    out_code,_temp_buf,sizeof(_zlib_TCompress)+kCompressBufSize);
        data_buf=_temp_buf+sizeof(_zlib_TCompress)+kCompressBufSize;
        if (!self) _compress_error_return("deflateInit2()");
        if (plugin->isNeedSaveWindowBits){
            const unsigned char* pchar=(const unsigned char*)&plugin->windowBits;
            if (!out_code->write(out_code,0,pchar,pchar+1)) _compress_error_return("out_code->write()");
            ++result;
        }
        while (readFromPos<in_data->streamSize){
            size_t readLen=kCompressBufSize;
            if (readLen>(hpatch_StreamPos_t)(in_data->streamSize-readFromPos))
                readLen=(size_t)(in_data->streamSize-readFromPos);
            if (!in_data->read(in_data,readFromPos,data_buf,data_buf+readLen))
                _compress_error_return("in_data->read()");
            readFromPos+=readLen;
            if (!_zlib_compress_part(self,data_buf,data_buf+readLen,
                                     (readFromPos==in_data->streamSize),&result,&outStream_isCanceled))
                _compress_error_return("_zlib_compress_part()");
        }
    clear:
        if (!_zlib_compress_close_by(compressPlugin,self))
            { result=kCompressFailResult; if (strlen(errAt)==0) errAt="deflateEnd()"; }
        _check_compress_result(result,outStream_isCanceled,"_zlib_compress()",errAt);
        if (_temp_buf) free(_temp_buf);
        return result;
    }
    _def_fun_compressType(_zlib_compressType,"zlib");
    static const TCompressPlugin_zlib zlibCompressPlugin={
        {_zlib_compressType,_default_maxCompressedSize,_default_setParallelThreadNumber,_zlib_compress},
            9,8,-MAX_WBITS,hpatch_TRUE,Z_DEFAULT_STRATEGY};
    
#   if (_IS_USED_MULTITHREAD)
    //pzlib
    typedef struct {
        TCompressPlugin_zlib base;
        int                  thread_num; // 1..
        hdiff_TParallelCompress pc;
    } TCompressPlugin_pzlib;
    static int _pzlib_setThreadNum(hdiff_TCompress* compressPlugin,int threadNum){
        TCompressPlugin_pzlib* plugin=(TCompressPlugin_pzlib*)compressPlugin;
        plugin->thread_num=threadNum;
        return threadNum;
    }
    static void _pzlib_closeBlockCompressor(hdiff_TParallelCompress* pc,
                                            hdiff_compressBlockHandle blockCompressor){
        z_stream* stream=(z_stream*)blockCompressor;
        if (!stream) return;
        if (stream->state!=0){
            int ret=deflateEnd(stream);
            assert((Z_OK==ret)|(Z_DATA_ERROR==ret));
        }
        free(stream);
    }
    static hdiff_compressBlockHandle _pzlib_openBlockCompressor(hdiff_TParallelCompress* pc){
        const TCompressPlugin_pzlib* plugin=(const TCompressPlugin_pzlib*)pc->import;
        z_stream* stream=(z_stream*)malloc(sizeof(z_stream));
        if (!stream) return 0;
        memset(stream,0,sizeof(z_stream));
        int err = deflateInit2(stream,plugin->base.compress_level,Z_DEFLATED,
                               plugin->base.windowBits,plugin->base.mem_level,Z_DEFAULT_STRATEGY);
        if (err!=Z_OK){//error
            _pzlib_closeBlockCompressor(pc,stream);
            return 0;
        }
        return stream;
    }
    static int _pzlib_compress2(Bytef* dest,uLongf* destLen,const unsigned char* block_data,
                                const unsigned char* block_dictEnd,const unsigned char* block_dataEnd,
                                int isEndBlock,z_stream* stream){
        int err;
        #define _check_zlib_err(_must_V) { if (err!=(_must_V)) goto _errorReturn; } 
        err=deflateReset(stream);
        _check_zlib_err(Z_OK);
        stream->next_in   = (Bytef*)block_dictEnd;
        stream->avail_in  = (uInt)(block_dataEnd-block_dictEnd);
        stream->next_out  = dest;
        stream->avail_out = (uInt)*destLen;
        stream->total_out = 0;
        if (block_data<block_dictEnd){
            err=deflateSetDictionary(stream,(Bytef*)block_data,(uInt)(block_dictEnd-block_data));
            _check_zlib_err(Z_OK);
        }
        if (!isEndBlock){
            int bits; 
            err = deflate(stream,Z_BLOCK);
            _check_zlib_err(Z_OK);
            // add enough empty blocks to get to a byte boundary
            err = deflatePending(stream,Z_NULL,&bits);
            _check_zlib_err(Z_OK);
            if (bits & 1){
                err = deflate(stream,Z_SYNC_FLUSH);
                _check_zlib_err(Z_OK);
            } else if (bits & 7) {
                do { // add static empty blocks
                    err = deflatePrime(stream, 10, 2);
                    _check_zlib_err(Z_OK);
                    err = deflatePending(stream,Z_NULL,&bits);
                    _check_zlib_err(Z_OK);
                } while (bits & 7);
                err = deflate(stream,Z_BLOCK);
                _check_zlib_err(Z_OK);
            }
        }else{
            err = deflate(stream,Z_FINISH);
            _check_zlib_err(Z_STREAM_END);
            err=Z_OK;
        }
        *destLen = stream->total_out;
        return err;
    _errorReturn:
        return err == Z_OK ? Z_BUF_ERROR : err;
        #undef _check_zlib_err
    }
    static
    size_t _pzlib_compressBlock(hdiff_TParallelCompress* pc,hdiff_compressBlockHandle blockCompressor,
                                hpatch_StreamPos_t blockIndex,hpatch_StreamPos_t blockCount,unsigned char* out_code,unsigned char* out_codeEnd,
                                const unsigned char* block_data,const unsigned char* block_dictEnd,const unsigned char* block_dataEnd){
        const TCompressPlugin_pzlib* plugin=(const TCompressPlugin_pzlib*)pc->import;
        const hpatch_BOOL isAdding=(blockIndex==0)&&(plugin->base.isNeedSaveWindowBits);
        if (isAdding){
            if (out_code>=out_codeEnd) return 0;//error;
            out_code[0]=plugin->base.windowBits;
            ++out_code;
        }
        uLongf codeLen=(uLongf)(out_codeEnd-out_code);
        if (Z_OK!=_pzlib_compress2(out_code,&codeLen,block_data,block_dictEnd,block_dataEnd,
                                   blockIndex+1==blockCount?1:0,(z_stream*)blockCompressor))
            return 0; //error
        return codeLen+(isAdding?1:0);
    }
    static hpatch_StreamPos_t _pzlib_compress(const hdiff_TCompress* compressPlugin,
                                              const hpatch_TStreamOutput* out_code,
                                              const hpatch_TStreamInput*  in_data){
        TCompressPlugin_pzlib* plugin=(TCompressPlugin_pzlib*)compressPlugin;
        const size_t blockSize=256*1024;
        if ((plugin->thread_num<=1)||(plugin->base.compress_level==0)
                ||(in_data->streamSize<blockSize*2)){ //same as "zlib"
            return _zlib_compress(compressPlugin,out_code,in_data);
        }else{
            int dictBits=plugin->base.windowBits;
            if (dictBits<0) dictBits=-dictBits;
            if (dictBits>15) dictBits-=16;
            if (dictBits<9) dictBits=9;
            else if (dictBits>15) dictBits=15;
            plugin->pc.import=plugin;
            return parallel_compress_blocks(&plugin->pc,plugin->thread_num,((size_t)1<<dictBits),blockSize,out_code,in_data);
        }
    }
    
    static const TCompressPlugin_pzlib pzlibCompressPlugin={
        { {_zlib_compressType,_default_maxCompressedSize,_pzlib_setThreadNum,_pzlib_compress},
            6,8,-MAX_WBITS,hpatch_TRUE,Z_DEFAULT_STRATEGY},
        kDefaultCompressThreadNumber ,{0,_default_maxCompressedSize,_pzlib_openBlockCompressor,
            _pzlib_closeBlockCompressor,_pzlib_compressBlock} };
#   endif // _IS_USED_MULTITHREAD
#endif//_CompressPlugin_zlib
*/
