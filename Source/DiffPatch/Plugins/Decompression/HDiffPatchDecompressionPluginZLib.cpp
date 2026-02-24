#include "HDiffPatchDecompressionPluginZLib.h"

#include <zlib.h>

/*
#ifdef  _CompressPlugin_zlib
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zlib.h" // http://zlib.net/  https://github.com/madler/zlib
#endif
    typedef struct _zlib_TDecompress{
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        const struct hpatch_TStreamInput* codeStream;
        
        unsigned char*  dec_buf;
        size_t          dec_buf_size;
        z_stream        d_stream;
        signed char     windowBits;
        hpatch_dec_error_t  decError;
    } _zlib_TDecompress;
    static void * __zlib_dec_Alloc(void* p,uInt items,uInt size) 
        __dec_Alloc_fun(_zlib_TDecompress,p,((items)*(size_t)(size)))
    static hpatch_BOOL _zlib_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"zlib"))||(0==strcmp(compressType,"pzlib"));
    }

    static _zlib_TDecompress*  _zlib_decompress_open_at(hpatch_TDecompress* decompressPlugin,
                                                        const hpatch_TStreamInput* codeStream,
                                                        hpatch_StreamPos_t code_begin,
                                                        hpatch_StreamPos_t code_end,
                                                        int  isSavedWindowBits,
                                                        _zlib_TDecompress* self,size_t _self_and_buf_size){
        int ret;
        signed char kWindowBits=-MAX_WBITS;
        assert(_self_and_buf_size>sizeof(_zlib_TDecompress));
        if (isSavedWindowBits){//load kWindowBits
            if (code_end-code_begin<1) _dec_openErr_rt();
            if (!codeStream->read(codeStream,code_begin,(unsigned char*)&kWindowBits,
                                  (unsigned char*)&kWindowBits+1)) return 0;
            ++code_begin;
        }
        
        memset(self,0,sizeof(_zlib_TDecompress));
        self->dec_buf=((unsigned char*)self)+sizeof(_zlib_TDecompress);
        self->dec_buf_size=_self_and_buf_size-sizeof(_zlib_TDecompress);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->windowBits=kWindowBits;
        self->d_stream.zalloc=__zlib_dec_Alloc;
        self->d_stream.zfree=__dec_free;
        self->d_stream.opaque=self;
        ret = inflateInit2(&self->d_stream,self->windowBits);
        if (ret!=Z_OK) { _dec_onDecErr_up(); _dec_openErr_rt(); }
        return self;
    }
    static hpatch_decompressHandle  _zlib_decompress_open(hpatch_TDecompress* decompressPlugin,
                                                          hpatch_StreamPos_t dataSize,
                                                          const hpatch_TStreamInput* codeStream,
                                                          hpatch_StreamPos_t code_begin,
                                                          hpatch_StreamPos_t code_end){
        _zlib_TDecompress* self=0;
        unsigned char* _mem_buf=(unsigned char*)_dec_malloc(sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!_mem_buf) _dec_memErr_rt();
        self=_zlib_decompress_open_at(decompressPlugin,codeStream,code_begin,code_end,1,
                                      (_zlib_TDecompress*)_mem_buf,sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!self)
            free(_mem_buf);
        return self;
    }
    static hpatch_decompressHandle  _zlib_decompress_open_deflate(hpatch_TDecompress* decompressPlugin,
                                                                  hpatch_StreamPos_t dataSize,
                                                                  const hpatch_TStreamInput* codeStream,
                                                                  hpatch_StreamPos_t code_begin,
                                                                  hpatch_StreamPos_t code_end){
        _zlib_TDecompress* self=0;
        unsigned char* _mem_buf=(unsigned char*)_dec_malloc(sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!_mem_buf) _dec_memErr_rt();
        self=_zlib_decompress_open_at(decompressPlugin,codeStream,code_begin,code_end,0,
                                      (_zlib_TDecompress*)_mem_buf,sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!self)
            free(_mem_buf);
        return self;
    }

    static _zlib_TDecompress*  _zlib_decompress_open_by(hpatch_TDecompress* decompressPlugin,
                                                        const hpatch_TStreamInput* codeStream,
                                                        hpatch_StreamPos_t code_begin,
                                                        hpatch_StreamPos_t code_end,
                                                        int  isSavedWindowBits,
                                                        unsigned char* _mem_buf,size_t _mem_buf_size){
        #define __MAX_TS(a,b)  ((a)>=(b)?(a):(b))
        const hpatch_size_t kZlibAlign=__MAX_TS(__MAX_TS(sizeof(hpatch_StreamPos_t),sizeof(void*)),sizeof(uLongf));
        #undef __MAX_TS
        unsigned char* _mem_buf_end=_mem_buf+_mem_buf_size;
        unsigned char* self_at=(unsigned char*)_hpatch_align_upper(_mem_buf,kZlibAlign);
        if (self_at>=_mem_buf_end) return 0;
        return _zlib_decompress_open_at(decompressPlugin,codeStream,code_begin,code_end,isSavedWindowBits,
                                        (_zlib_TDecompress*)self_at,_mem_buf_end-self_at);
    }
    static hpatch_BOOL _zlib_decompress_close_by(struct hpatch_TDecompress* decompressPlugin,
                                                 _zlib_TDecompress* self){
        hpatch_BOOL result=hpatch_TRUE;
        if (!self) return result;
        _dec_onDecErr_up();
        if (self->d_stream.state!=0){
            _dec_close_check(Z_OK==inflateEnd(&self->d_stream));
        }
        memset(self,0,sizeof(_zlib_TDecompress));
        return result;
    }
    
    static hpatch_BOOL _zlib_decompress_close(struct hpatch_TDecompress* decompressPlugin,
                                              hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        hpatch_BOOL result=_zlib_decompress_close_by(decompressPlugin,self);
        if (self) free(self);
        return result;
    }

        static hpatch_BOOL _zlib_reset_for_next_node(z_stream* d_stream){
            //backup
            Bytef*   next_out_back=d_stream->next_out;
            Bytef*   next_in_back=d_stream->next_in;
            unsigned int avail_out_back=d_stream->avail_out;
            unsigned int avail_in_back=d_stream->avail_in;
                //reset
            if (Z_OK!=inflateReset(d_stream)) return hpatch_FALSE;
                //restore
            d_stream->next_out=next_out_back;
            d_stream->next_in=next_in_back;
            d_stream->avail_out=avail_out_back;
            d_stream->avail_in=avail_in_back;
            return hpatch_TRUE;
        }
    static hpatch_BOOL __zlib_do_inflate(hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        uInt avail_out_back,avail_in_back;
        int ret;
        hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
        if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
            size_t readLen=self->dec_buf_size;
            if (readLen>codeLen) readLen=(size_t)codeLen;
            self->d_stream.next_in=self->dec_buf;
            if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf,
                                        self->dec_buf+readLen)) return hpatch_FALSE;//error;
            self->d_stream.avail_in=(uInt)readLen;
            self->code_begin+=readLen;
            codeLen-=readLen;
        }
        
        avail_out_back=self->d_stream.avail_out;
        avail_in_back=self->d_stream.avail_in;
        ret=inflate(&self->d_stream,Z_NO_FLUSH);
        if (ret==Z_OK){
            if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                _dec_onDecErr_rt();//error;
        }else if (ret==Z_STREAM_END){
            if (self->d_stream.avail_in+codeLen>0){ //next compress node!
                if (!_zlib_reset_for_next_node(&self->d_stream))
                    _dec_onDecErr_rt();//error;
            }else{//all end
                if (self->d_stream.avail_out!=0)
                    _dec_onDecErr_rt();//error;
            }
        }else{
            _dec_onDecErr_rt();//error;
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _zlib_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        assert(out_part_data<=out_part_data_end);
        
        self->d_stream.next_out = out_part_data;
        self->d_stream.avail_out =(uInt)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            if (!__zlib_do_inflate(self))
                return hpatch_FALSE;//error;
        }
        return hpatch_TRUE;
    }
    static hpatch_inline int _zlib_is_decompress_finish(const hpatch_TDecompress* decompressPlugin,
                                                        hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        unsigned char _empty=0;
        while (self->code_begin!=self->code_end){ //for end tag code
            self->d_stream.next_out = &_empty;
            self->d_stream.avail_out=0;
            if (!__zlib_do_inflate(self)){
                self->d_stream.next_out=0;
                return hpatch_FALSE;//error;
            }
        }
        self->d_stream.next_out=0;
        return   (self->code_begin==self->code_end)
                &(self->d_stream.avail_in==0)
                &(self->d_stream.avail_out==0);
    }
    static hpatch_TDecompress zlibDecompressPlugin={_zlib_is_can_open,_zlib_decompress_open,
                                                    _zlib_decompress_close,_zlib_decompress_part};
    static hpatch_TDecompress zlibDecompressPlugin_deflate={_zlib_is_can_open,_zlib_decompress_open_deflate,
                                                    _zlib_decompress_close,_zlib_decompress_part};
#endif//_CompressPlugin_zlib
*/
