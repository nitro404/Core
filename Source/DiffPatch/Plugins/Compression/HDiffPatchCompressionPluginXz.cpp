#include "HDiffPatchCompressionPluginXz.h"

#include <lzma.h>

/*
#ifdef  _CompressPlugin_7zXZ
#if (_IsNeedIncludeDefaultCompressHead)
#   include <SevenZip/C/XzEnc.h>
#   include <SevenZip/C/MtCoder.h> // for MTCODER__THREADS_MAX
#   include <SevenZip/C/7zCrc.h> // CrcGenerateTable()
#endif

#ifndef _init_CompressPlugin_7zXZ_DEF
#   define _init_CompressPlugin_7zXZ_DEF
    static int _init_CompressPlugin_7zXZ(){
        static hpatch_BOOL _isInit=hpatch_FALSE;
        if (!_isInit){
            CrcGenerateTable();
            _isInit=hpatch_TRUE;
        }
        return 0;
    }
#endif

    typedef struct{
        hdiff_TCompress base;
        int             compress_level; //0..9
        UInt32          dict_size;      //patch decompress need 3?*lzma_dictSize memory
        int             thread_num;     //1..(64?)
    } TCompressPlugin_7zXZ;
    static int _7zXZ_setThreadNumber(hdiff_TCompress* compressPlugin,int threadNum){
        TCompressPlugin_lzma2* plugin=(TCompressPlugin_lzma2*)compressPlugin;
        if (threadNum>MTCODER_THREADS_MAX) threadNum=MTCODER_THREADS_MAX;
        plugin->thread_num=threadNum;
        return threadNum;
    }
    
    typedef void* _7zXZ_compressHandle;
    static _7zXZ_compressHandle _7zXZ_compress_open(const hdiff_TCompress* compressPlugin){
        const TCompressPlugin_7zXZ* plugin=(const TCompressPlugin_7zXZ*)compressPlugin;
        _7zXZ_compressHandle result=0;
        const char*        errAt="";
#if (IS_REUSE_compress_handle)
        static CXzEncHandle  s=0;
#else
        CXzEncHandle         s=0;
#endif
        CXzProps           xzprops;
        hpatch_uint32_t    dictSize=plugin->dict_size;
        if (!s) s=XzEnc_Create(&__lzma_enc_alloc,&__lzma_enc_alloc);
        if (!s) _compress_error_return("XzEnc_Create()");
        XzProps_Init(&xzprops);
        xzprops.lzma2Props.lzmaProps.level=plugin->compress_level;
        xzprops.lzma2Props.lzmaProps.dictSize=dictSize;
        xzprops.lzma2Props.numTotalThreads=plugin->thread_num;
        Lzma2EncProps_Normalize(&xzprops.lzma2Props);
        xzprops.numTotalThreads=plugin->thread_num;
        xzprops.checkId=XZ_CHECK_NO;
        if (SZ_OK!=XzEnc_SetProps(s,&xzprops)) _compress_error_return("XzEnc_SetProps()");
        return s;
    clear:
#if (!IS_REUSE_compress_handle)
        if (s) { XzEnc_Destroy(s); s=0; }
#endif
        return result;
    }
    static hpatch_BOOL _7zXZ_compress_close(const hdiff_TCompress* compressPlugin,
                                     _7zXZ_compressHandle compressHandle){
#if (!IS_REUSE_compress_handle)
        CXzEncHandle s=(CXzEncHandle)compressHandle;
        if (s) { XzEnc_Destroy(s); s=0; }
#endif
        return hpatch_TRUE;
    }

    static hpatch_StreamPos_t _7zXZ_compress_encode(_7zXZ_compressHandle compressHandle,
                                                    const hpatch_TStreamOutput* out_code,
                                                    const hpatch_TStreamInput*  in_data,
                                                    hpatch_BOOL isWriteHead,hpatch_BOOL isWriteEnd){
        struct __lzma_SeqOutStream_t outStream={{__lzma_SeqOutStream_Write},out_code,0,0};
        struct __lzma_SeqInStream_t  inStream={{__lzma_SeqInStream_Read},in_data,0};
        hpatch_StreamPos_t result=0;
        const char*        errAt="";
        CXzEncHandle s=(CXzEncHandle)compressHandle;
        SRes               ret;
        XzEnc_SetDataSize(s,in_data->streamSize);
        
        ret=XzEnc_Encode_Part(s,&outStream.base,&inStream.base,0,isWriteHead,isWriteEnd);
        if (SZ_OK==ret){
            result=outStream.writeToPos;
        }else{//fail
            if (ret==SZ_ERROR_READ)
                _compress_error_return("in_data->read()");
            else if (ret==SZ_ERROR_WRITE)
                _compress_error_return("out_code->write()");
            else
                _compress_error_return("XzEnc_Encode_Part()");
        }
    clear:
        _check_compress_result(result,outStream.isCanceled,"_7zXZ_compress_encode()",errAt);
        return result;
    }

    static hpatch_StreamPos_t _7zXZ_compress(const hdiff_TCompress* compressPlugin,
                                             const hpatch_TStreamOutput* out_code,
                                             const hpatch_TStreamInput*  in_data){
        const TCompressPlugin_7zXZ* plugin=(const TCompressPlugin_7zXZ*)compressPlugin;
        hpatch_StreamPos_t result=0;
        const char*        errAt="";
        _7zXZ_compressHandle s=_7zXZ_compress_open(compressPlugin);
        if (!s) _compress_error_return("_7zXZ_compress_open()");
        result=_7zXZ_compress_encode(s,out_code,in_data,hpatch_TRUE,hpatch_TRUE);
        if (result==0) 
            _compress_error_return("_7zXZ_compress_encode()");
    clear:
        if (s) { _7zXZ_compress_close(compressPlugin,s); s=0; }
        return result;
    }
    _def_fun_compressType(_7zXZ_compressType,"7zXZ");
    static const TCompressPlugin_7zXZ _7zXZCompressPlugin={
        {_7zXZ_compressType,_default_maxCompressedSize,_7zXZ_setThreadNumber,_7zXZ_compress},
        7,(1<<23),kDefaultCompressThreadNumber};

#endif //_CompressPlugin_7zXZ
*/
