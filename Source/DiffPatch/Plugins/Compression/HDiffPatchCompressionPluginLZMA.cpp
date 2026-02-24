#include "HDiffPatchCompressionPluginLZMA.h"

#include <lzma.h>

/*
#if (defined _CompressPlugin_lzma)||(defined _CompressPlugin_lzma2)||(defined _CompressPlugin_7zXZ)
#if (_IsNeedIncludeDefaultCompressHead)
#   include <SevenZip/C/LzmaEnc.h>
#   ifdef _CompressPlugin_lzma2
#       include <SevenZip/C/Lzma2Enc.h>
#   endif
#endif
    static void * __lzma_enc_Alloc(ISzAllocPtr p, size_t size){
        return malloc(size);
    }
    static void __lzma_enc_Free(ISzAllocPtr p, void *address){
        free(address);
    }
    static ISzAlloc __lzma_enc_alloc={__lzma_enc_Alloc,__lzma_enc_Free};
    
    struct __lzma_SeqOutStream_t{
        ISeqOutStream               base;
        const hpatch_TStreamOutput* out_code;
        hpatch_StreamPos_t          writeToPos;
        int                         isCanceled;
    };
    static size_t __lzma_SeqOutStream_Write(const ISeqOutStream *p, const void *buf, size_t size){
        __lzma_SeqOutStream_t* self=(__lzma_SeqOutStream_t*)p;
        const unsigned char* pdata=(const unsigned char*)buf;
        if (size>0){
            if (!self->out_code->write(self->out_code,self->writeToPos,pdata,pdata+size)){
                self->isCanceled=1;
                return 0;
            }
        }
        self->writeToPos+=size;
        return size;
    }
    struct __lzma_SeqInStream_t{
        ISeqInStream                base;
        const hpatch_TStreamInput*  in_data;
        hpatch_StreamPos_t          readFromPos;
    };
    static SRes __lzma_SeqInStream_Read(const ISeqInStream *p, void *buf, size_t *size){
        __lzma_SeqInStream_t* self=(__lzma_SeqInStream_t*)p;
        size_t readLen=*size;
        if (readLen+self->readFromPos>self->in_data->streamSize)
            readLen=(size_t)(self->in_data->streamSize-self->readFromPos);
        if (readLen>0){
            unsigned char* pdata=(unsigned char*)buf;
            if (!self->in_data->read(self->in_data,self->readFromPos,pdata,pdata+readLen)){
                *size=0;
                return SZ_ERROR_READ;
            }
        }
        self->readFromPos+=readLen;
        *size=readLen;
        return SZ_OK;
    }
#endif
    
#ifdef  _CompressPlugin_lzma
    typedef struct{
        hdiff_TCompress base;
        int             compress_level; //0..9
        UInt32          dict_size;      //patch decompress need 4?*lzma_dictSize memory
        int             thread_num;     //1..2
    } TCompressPlugin_lzma;
    static int _lzma_setThreadNumber(hdiff_TCompress* compressPlugin,int threadNum){
        TCompressPlugin_lzma* plugin=(TCompressPlugin_lzma*)compressPlugin;
        if (threadNum>2) threadNum=2;
        plugin->thread_num=threadNum;
        return threadNum;
    }
    static hpatch_StreamPos_t _lzma_compress(const hdiff_TCompress* compressPlugin,
                                             const hpatch_TStreamOutput* out_code,
                                             const hpatch_TStreamInput*  in_data){
        const TCompressPlugin_lzma* plugin=(const TCompressPlugin_lzma*)compressPlugin;
        struct __lzma_SeqOutStream_t outStream={{__lzma_SeqOutStream_Write},out_code,0,0};
        struct __lzma_SeqInStream_t  inStream={{__lzma_SeqInStream_Read},in_data,0};
        hpatch_StreamPos_t result=0;
        const char*        errAt="";
#if (IS_REUSE_compress_handle)
        static CLzmaEncHandle   s=0;
#else
        CLzmaEncHandle          s=0;
#endif
        CLzmaEncProps      props;
        unsigned char      properties_buf[LZMA_PROPS_SIZE+1];
        SizeT              properties_size=LZMA_PROPS_SIZE;
        SRes               ret;
        hpatch_uint32_t    dictSize=plugin->dict_size;
        if (!s) s=LzmaEnc_Create(&__lzma_enc_alloc);
        if (!s) _compress_error_return("LzmaEnc_Create()");
        LzmaEncProps_Init(&props);
        props.level=plugin->compress_level;
        props.dictSize=dictSize;
        props.reduceSize=in_data->streamSize;
        props.numThreads=plugin->thread_num;
        LzmaEncProps_Normalize(&props);
        if (SZ_OK!=LzmaEnc_SetProps(s,&props)) _compress_error_return("LzmaEnc_SetProps()");
#       if (IS_NOTICE_compress_canceled)
        printf("    (used one lzma dictSize: %" PRIu64 "  (input data: %" PRIu64 "))\n",
               (hpatch_StreamPos_t)props.dictSize,in_data->streamSize);
#       endif
        
        //save properties_size+properties
        assert(LZMA_PROPS_SIZE<256);
        if (SZ_OK!=LzmaEnc_WriteProperties(s,properties_buf+1,&properties_size))
            _compress_error_return("LzmaEnc_WriteProperties()");
        properties_buf[0]=(unsigned char)properties_size;
        
        if (1+properties_size!=__lzma_SeqOutStream_Write(&outStream.base,properties_buf,1+properties_size))
            _compress_error_return("out_code->write()");
        
        ret=LzmaEnc_Encode(s,&outStream.base,&inStream.base,0,&__lzma_enc_alloc,&__lzma_enc_alloc);
        if (SZ_OK==ret){
            result=outStream.writeToPos;
        }else{//fail
            if (ret==SZ_ERROR_READ)
                _compress_error_return("in_data->read()");
            else if (ret==SZ_ERROR_WRITE)
                _compress_error_return("out_code->write()");
            else
                _compress_error_return("LzmaEnc_Encode()");
        }
    clear:
#if (!IS_REUSE_compress_handle)
        if (s) { LzmaEnc_Destroy(s,&__lzma_enc_alloc,&__lzma_enc_alloc); s=0; }
#endif
        _check_compress_result(result,outStream.isCanceled,"_lzma_compress()",errAt);
        return result;
    }
    _def_fun_compressType(_lzma_compressType,"lzma");
    static const TCompressPlugin_lzma lzmaCompressPlugin={
        {_lzma_compressType,_default_maxCompressedSize,_lzma_setThreadNumber,_lzma_compress},
        7,(1<<23),(kDefaultCompressThreadNumber>=2)?2:kDefaultCompressThreadNumber};
#endif//_CompressPlugin_lzma
    
#ifdef  _CompressPlugin_lzma2
#if (_IsNeedIncludeDefaultCompressHead)
#   include <SevenZip/C/MtCoder.h> // "lzma/C/MtCoder.h"   for MTCODER__THREADS_MAX
#endif
    struct TCompressPlugin_lzma2{
        hdiff_TCompress base;
        int             compress_level; //0..9
        UInt32          dict_size;      //patch decompress need 4?*lzma_dictSize memory
        int             thread_num;     //1..(64?)
    };
    static int _lzma2_setThreadNumber(hdiff_TCompress* compressPlugin,int threadNum){
        TCompressPlugin_lzma2* plugin=(TCompressPlugin_lzma2*)compressPlugin;
        if (threadNum>MTCODER_THREADS_MAX) threadNum=MTCODER_THREADS_MAX;
        plugin->thread_num=threadNum;
        return threadNum;
    }
    static hpatch_StreamPos_t _lzma2_compress(const hdiff_TCompress* compressPlugin,
                                              const hpatch_TStreamOutput* out_code,
                                              const hpatch_TStreamInput*  in_data){
        const TCompressPlugin_lzma2* plugin=(const TCompressPlugin_lzma2*)compressPlugin;
        struct __lzma_SeqOutStream_t outStream={{__lzma_SeqOutStream_Write},out_code,0,0};
        struct __lzma_SeqInStream_t  inStream={{__lzma_SeqInStream_Read},in_data,0};
        hpatch_StreamPos_t result=0;
        const char*        errAt="";
#if (IS_REUSE_compress_handle)
        static CLzma2EncHandle  s=0;
#else
        CLzma2EncHandle         s=0;
#endif
        CLzma2EncProps     props;
        Byte               properties_size=0;
        SRes               ret;
        hpatch_uint32_t    dictSize=plugin->dict_size;
        if (!s) s=Lzma2Enc_Create(&__lzma_enc_alloc,&__lzma_enc_alloc);
        if (!s) _compress_error_return("LzmaEnc_Create()");
        Lzma2EncProps_Init(&props);
        props.lzmaProps.level=plugin->compress_level;
        props.lzmaProps.dictSize=dictSize;
        props.lzmaProps.reduceSize=in_data->streamSize;
        props.blockSize=LZMA2_ENC_PROPS_BLOCK_SIZE_AUTO;
        props.numTotalThreads=plugin->thread_num;
        Lzma2EncProps_Normalize(&props);
        if (SZ_OK!=Lzma2Enc_SetProps(s,&props)) _compress_error_return("Lzma2Enc_SetProps()");
#       if (IS_NOTICE_compress_canceled)
        printf("    (used one lzma2 dictSize: %" PRIu64 "  (input data: %" PRIu64 "))\n",
               (hpatch_StreamPos_t)props.lzmaProps.dictSize,in_data->streamSize);
#       endif
        
        //save properties_size+properties
        assert(LZMA_PROPS_SIZE<256);
        properties_size=Lzma2Enc_WriteProperties(s);
        
        if (1!=__lzma_SeqOutStream_Write(&outStream.base,&properties_size,1))
            _compress_error_return("out_code->write()");
        
        ret=Lzma2Enc_Encode2(s,&outStream.base,0,0,&inStream.base,0,0,0);
        if (SZ_OK==ret){
            result=outStream.writeToPos;
        }else{//fail
            if (ret==SZ_ERROR_READ)
                _compress_error_return("in_data->read()");
            else if (ret==SZ_ERROR_WRITE)
                _compress_error_return("out_code->write()");
            else
                _compress_error_return("Lzma2Enc_Encode2()");
        }
    clear:
#if (!IS_REUSE_compress_handle)
        if (s) { Lzma2Enc_Destroy(s); s=0; }
#endif
        _check_compress_result(result,outStream.isCanceled,"_lzma2_compress()",errAt);
        return result;
    }
    _def_fun_compressType(_lzma2_compressType,"lzma2");
    static const TCompressPlugin_lzma2 lzma2CompressPlugin={
        {_lzma2_compressType,_default_maxCompressedSize,_lzma2_setThreadNumber,_lzma2_compress},
        7,(1<<23),kDefaultCompressThreadNumber};
#endif//_CompressPlugin_lzma2
*/
