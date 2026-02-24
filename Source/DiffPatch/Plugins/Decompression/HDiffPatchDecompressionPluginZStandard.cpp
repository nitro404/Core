#include "HDiffPatchDecompressionPluginZStandard.h"

#include <zstd.h>

/*
#ifdef  _CompressPlugin_zstd
#if (_IsNeedIncludeDefaultCompressHead)
//#   define ZSTD_STATIC_LINKING_ONLY //for ZSTD_customMem
#   include "zstd.h" // "zstd/lib/zstd.h" https://github.com/sisong/zstd
#endif
    typedef struct _zstd_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        ZSTD_inBuffer      s_input;
        ZSTD_outBuffer     s_output;
        size_t             data_begin;
        ZSTD_DStream*      s;
        hpatch_dec_error_t decError;
        unsigned char      buf[1];
    } _zstd_TDecompress;
    #ifdef ZSTD_STATIC_LINKING_ONLY
    static void* __ZSTD_alloc(void* opaque, size_t size)
        __dec_Alloc_fun(_zstd_TDecompress,opaque,size)
    #endif
    static hpatch_BOOL _zstd_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"zstd"));
    }
    static hpatch_decompressHandle  _zstd_open(hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _zstd_TDecompress* self=0;
        size_t  ret;
        size_t _input_size=ZSTD_DStreamInSize();
        size_t _output_size=ZSTD_DStreamOutSize();
        self=(_zstd_TDecompress*)_dec_malloc(sizeof(_zstd_TDecompress)+_input_size+_output_size);
        if (!self) _dec_memErr_rt();
        memset(self,0,sizeof(_zstd_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->s_input.src=self->buf;
        self->s_input.size=_input_size;
        self->s_input.pos=_input_size;
        self->s_output.dst=self->buf+_input_size;
        self->s_output.size=_output_size;
        self->s_output.pos=0;
        self->data_begin=0;
        #ifdef ZSTD_STATIC_LINKING_ONLY
        {
            ZSTD_customMem customMem={__ZSTD_alloc,__dec_free,self};
            self->s=ZSTD_createDStream_advanced(customMem);
        }
        #else
            self->s=ZSTD_createDStream();
        #endif
        if (!self->s){ _dec_onDecErr_up(); free(self); _dec_openErr_rt(); }
        ret=ZSTD_initDStream(self->s);
        if (ZSTD_isError(ret)) { ZSTD_freeDStream(self->s); _dec_onDecErr_up(); free(self); _dec_openErr_rt(); }
        #define _ZSTD_WINDOWLOG_MAX 30
        ret=ZSTD_DCtx_setParameter(self->s,ZSTD_d_windowLogMax,_ZSTD_WINDOWLOG_MAX);
        //if (ZSTD_isError(ret)) { printf("WARNING: ZSTD_DCtx_setMaxWindowSize() error!"); }
        return self;
    }
    static hpatch_BOOL _zstd_close(struct hpatch_TDecompress* decompressPlugin,
                                   hpatch_decompressHandle decompressHandle){
        hpatch_BOOL result=hpatch_TRUE;
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        if (!self) return result;
        _dec_onDecErr_up();
        _dec_close_check(0==ZSTD_freeDStream(self->s));
        free(self);
        return result;
    }
    static hpatch_BOOL _zstd_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        while (out_part_data<out_part_data_end) {
            size_t dataLen=(self->s_output.pos-self->data_begin);
            if (dataLen>0){
                if (dataLen>(size_t)(out_part_data_end-out_part_data))
                    dataLen=(out_part_data_end-out_part_data);
                memcpy(out_part_data,(const unsigned char*)self->s_output.dst+self->data_begin,dataLen);
                out_part_data+=dataLen;
                self->data_begin+=dataLen;
            }else{
                size_t ret;
                if (self->s_input.pos==self->s_input.size) {
                    self->s_input.pos=0;
                    if (self->s_input.size>self->code_end-self->code_begin)
                        self->s_input.size=(size_t)(self->code_end-self->code_begin);

                    if (self->s_input.size>0){
                        if (!self->codeStream->read(self->codeStream,self->code_begin,(unsigned char*)self->s_input.src,
                                                    (unsigned char*)self->s_input.src+self->s_input.size))
                            return hpatch_FALSE;
                        self->code_begin+=self->s_input.size;
                    }
                }
                self->s_output.pos=0;
                self->data_begin=0;
                ret=ZSTD_decompressStream(self->s,&self->s_output,&self->s_input);
                if (ZSTD_isError(ret)) _dec_onDecErr_rt();
                if (self->s_output.pos==self->data_begin) _dec_onDecErr_rt();
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress zstdDecompressPlugin={_zstd_is_can_open,_zstd_open,
                                                    _zstd_close,_zstd_decompress_part};
#endif//_CompressPlugin_zstd
*/
