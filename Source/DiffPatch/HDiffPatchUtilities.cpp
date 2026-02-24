#include "HDiffPatchUtilities.h"

/*
static hpatch_TDecompress* __find_decompressPlugin(const char* compressType){
#if ((defined(_CompressPlugin_ldef))&&_IS_NEED_decompressor_ldef_replace_zlib)
    _try_rt_dec(ldefDecompressPlugin);
#else
#  ifdef  _CompressPlugin_zlib
    _try_rt_dec(zlibDecompressPlugin);
#  endif
#endif
#ifdef  _CompressPlugin_bz2
    _try_rt_dec(bz2DecompressPlugin);
#endif
#ifdef  _CompressPlugin_lzma
    _try_rt_dec(lzmaDecompressPlugin);
#endif
#ifdef  _CompressPlugin_lzma2
    _try_rt_dec(lzma2DecompressPlugin);
#endif
#if (defined(_CompressPlugin_lz4) || (defined(_CompressPlugin_lz4hc)))
    _try_rt_dec(lz4DecompressPlugin);
#endif
#ifdef  _CompressPlugin_zstd
    _try_rt_dec(zstdDecompressPlugin);
#endif
#ifdef  _CompressPlugin_brotli
    _try_rt_dec(brotliDecompressPlugin);
#endif
#ifdef  _CompressPlugin_lzham
    _try_rt_dec(lzhamDecompressPlugin);
#endif
#ifdef  _CompressPlugin_tuz
    _try_rt_dec(tuzDecompressPlugin);
#endif
    return 0;
}
*/

/*
#define __getCompressSet(_tryGet_code,_errTag)  \
    if (isMatchedType==0){                      \
        _options_check(_tryGet_code,_errTag);   \
        if (isMatchedType)

static bool _tryGetCompressSet(const char** isMatchedType,const char* ptype,const char* ptypeEnd,
                               const char* cmpType,const char* cmpType2=0,
                               size_t* compressLevel=0,size_t levelMin=0,size_t levelMax=0,size_t levelDefault=0,
                               size_t* dictSize=0,size_t dictSizeMin=0,size_t dictSizeMax=0,size_t dictSizeDefault=0){
    assert (0==(*isMatchedType));
    const size_t ctypeLen=strlen(cmpType);
    const size_t ctype2Len=(cmpType2!=0)?strlen(cmpType2):0;
    if ( ((ctypeLen==(size_t)(ptypeEnd-ptype))&&(0==strncmp(ptype,cmpType,ctypeLen)))
        || ((cmpType2!=0)&&(ctype2Len==(size_t)(ptypeEnd-ptype))&&(0==strncmp(ptype,cmpType2,ctype2Len))) )
        *isMatchedType=cmpType; //ok
    else
        return true;//type mismatch
    
    if ((compressLevel)&&(ptypeEnd[0]=='-')){
        const char* plevel=ptypeEnd+1;
        const char* plevelEnd=findUntilEnd(plevel,'-');
        if (!kmg_to_size(plevel,plevelEnd-plevel,compressLevel)) return false; //error
        if (*compressLevel<levelMin) *compressLevel=levelMin;
        else if (*compressLevel>levelMax) *compressLevel=levelMax;
        if ((dictSize)&&(plevelEnd[0]=='-')){
            const char* pdictSize=plevelEnd+1;
            const char* pdictSizeEnd=findUntilEnd(pdictSize,'-');
            if (!kmg_to_size(pdictSize,pdictSizeEnd-pdictSize,dictSize)) return false; //error
            if (*dictSize<dictSizeMin) *dictSize=dictSizeMin;
            else if (*dictSize>dictSizeMax) *dictSize=dictSizeMax;
        }else{
            if (plevelEnd[0]!='\0') return false; //error
            if (dictSize) *dictSize=(dictSizeDefault<dictSizeMax)?dictSizeDefault:dictSizeMax;
        }
    }else{
        if (ptypeEnd[0]!='\0') return false; //error
        if (compressLevel) *compressLevel=levelDefault;
        if (dictSize) *dictSize=dictSizeDefault;
    }
    return true;
}

static int _checkSetCompress(hdiff_TCompress** out_compressPlugin,
                             const char* ptype,const char* ptypeEnd){
    const char* isMatchedType=0;
    size_t      compressLevel=0;
#if (defined _CompressPlugin_lzma)||(defined _CompressPlugin_lzma2)||(defined _CompressPlugin_tuz)
    size_t       dictSize=0;
    const size_t defaultDictSize=(1<<20)*8; //8m
#endif
#if (defined _CompressPlugin_zlib)||(defined _CompressPlugin_ldef)||(defined _CompressPlugin_zstd) \
        ||(defined _CompressPlugin_brotli)||(defined _CompressPlugin_lzham)
    size_t       dictBits=0;
    const size_t defaultDictBits=20+3; //8m
    const size_t defaultDictBits_zlib=15; //32k
#endif
#ifdef _CompressPlugin_zlib
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"zlib","pzlib",
                                        &compressLevel,1,9,9, &dictBits,9,15,defaultDictBits_zlib),"-c-zlib-?"){
#   if (!_IS_USED_MULTITHREAD)
        static TCompressPlugin_zlib _zlibCompressPlugin=zlibCompressPlugin;
        _zlibCompressPlugin.compress_level=(int)compressLevel;
        _zlibCompressPlugin.windowBits=(signed char)(-dictBits);
        *out_compressPlugin=&_zlibCompressPlugin.base; }}
#   else
        static TCompressPlugin_pzlib _pzlibCompressPlugin=pzlibCompressPlugin;
        _pzlibCompressPlugin.base.compress_level=(int)compressLevel;
        _pzlibCompressPlugin.base.windowBits=(signed char)(-(int)dictBits);
        *out_compressPlugin=&_pzlibCompressPlugin.base.base; }}
#   endif // _IS_USED_MULTITHREAD
#endif
#ifdef _CompressPlugin_ldef
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"ldef","pldef",
                                        &compressLevel,1,12,12, &dictBits,15,15,defaultDictBits_zlib),"-c-ldef-?"){
#   if (!_IS_USED_MULTITHREAD)
        static TCompressPlugin_ldef _ldefCompressPlugin=ldefCompressPlugin;
        _ldefCompressPlugin.compress_level=(int)compressLevel;
        *out_compressPlugin=&_ldefCompressPlugin.base; }}
#   else
        static TCompressPlugin_pldef _pldefCompressPlugin=pldefCompressPlugin;
        _pldefCompressPlugin.base.compress_level=(int)compressLevel;
        *out_compressPlugin=&_pldefCompressPlugin.base.base; }}
#   endif // _IS_USED_MULTITHREAD
#endif
#ifdef _CompressPlugin_bz2
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"bzip2","bz2",
                                        &compressLevel,1,9,9),"-c-bzip2-?"){
        static TCompressPlugin_bz2 _bz2CompressPlugin=bz2CompressPlugin;
        _bz2CompressPlugin.compress_level=(int)compressLevel;
        *out_compressPlugin=&_bz2CompressPlugin.base; }}
#   if (_IS_USED_MULTITHREAD)
    //pbzip2
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"pbzip2","pbz2",
                                        &compressLevel,1,9,8),"-c-pbzip2-?"){
        static TCompressPlugin_pbz2 _pbz2CompressPlugin=pbz2CompressPlugin;
        _pbz2CompressPlugin.base.compress_level=(int)compressLevel;
        *out_compressPlugin=&_pbz2CompressPlugin.base.base; }}
#   endif // _IS_USED_MULTITHREAD
#endif
#ifdef _CompressPlugin_lzma
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"lzma",0,
                                        &compressLevel,0,9,7, &dictSize,1<<12,
                                        (sizeof(size_t)<=4)?(1<<27):((size_t)3<<29),defaultDictSize),"-c-lzma-?"){
        static TCompressPlugin_lzma _lzmaCompressPlugin=lzmaCompressPlugin;
        _lzmaCompressPlugin.compress_level=(int)compressLevel;
        _lzmaCompressPlugin.dict_size=(UInt32)dictSize;
        *out_compressPlugin=&_lzmaCompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_lzma2
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"lzma2",0,
                                        &compressLevel,0,9,7, &dictSize,1<<12,
                                        (sizeof(size_t)<=4)?(1<<27):((size_t)3<<29),defaultDictSize),"-c-lzma2-?"){
        static TCompressPlugin_lzma2 _lzma2CompressPlugin=lzma2CompressPlugin;
        _lzma2CompressPlugin.compress_level=(int)compressLevel;
        _lzma2CompressPlugin.dict_size=(UInt32)dictSize;
        *out_compressPlugin=&_lzma2CompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_lz4
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"lz4",0,
                                        &compressLevel,1,50,50),"-c-lz4-?"){
        static TCompressPlugin_lz4 _lz4CompressPlugin=lz4CompressPlugin;
        _lz4CompressPlugin.compress_level=(int)compressLevel;
        *out_compressPlugin=&_lz4CompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_lz4hc
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"lz4hc",0,
                                        &compressLevel,3,12,11),"-c-lz4hc-?"){
        static TCompressPlugin_lz4hc _lz4hcCompressPlugin=lz4hcCompressPlugin;
        _lz4hcCompressPlugin.compress_level=(int)compressLevel;
        *out_compressPlugin=&_lz4hcCompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_zstd
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"zstd",0,
                                        &compressLevel,0,22,20, &dictBits,10,
                                        _ZSTD_WINDOWLOG_MAX,defaultDictBits),"-c-zstd-?"){
        static TCompressPlugin_zstd _zstdCompressPlugin=zstdCompressPlugin;
        _zstdCompressPlugin.compress_level=(int)compressLevel;
        _zstdCompressPlugin.dict_bits = (int)dictBits;
        *out_compressPlugin=&_zstdCompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_brotli
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"brotli",0,
                                        &compressLevel,0,11,9, &dictBits,10,
                                        30,defaultDictBits),"-c-brotli-?"){
        static TCompressPlugin_brotli _brotliCompressPlugin=brotliCompressPlugin;
        _brotliCompressPlugin.compress_level=(int)compressLevel;
        _brotliCompressPlugin.dict_bits = (int)dictBits;
        *out_compressPlugin=&_brotliCompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_lzham
    __getCompressSet(_tryGetCompressSet(&isMatchedType,ptype,ptypeEnd,"lzham",0,
                                        &compressLevel,0,5,4, &dictBits,15,
                                        (sizeof(size_t)<=4)?26:29,defaultDictBits),"-c-lzham-?"){
        static TCompressPlugin_lzham _lzhamCompressPlugin=lzhamCompressPlugin;
        _lzhamCompressPlugin.compress_level=(int)compressLevel;
        _lzhamCompressPlugin.dict_bits = (int)dictBits;
        *out_compressPlugin=&_lzhamCompressPlugin.base; }}
#endif
#ifdef _CompressPlugin_tuz
    __getCompressSet(_tryGetCompressSet(&isMatchedType,
                                        ptype,ptypeEnd,"tuz","tinyuz",
                                        &dictSize,1,tuz_kMaxOfDictSize,defaultDictSize),"-c-tuz-?"){
        static TCompressPlugin_tuz _tuzCompressPlugin=tuzCompressPlugin;
        _tuzCompressPlugin.props.dictSize=(tuz_size_t)dictSize;
        *out_compressPlugin=&_tuzCompressPlugin.base; }}
#endif

    _options_check((*out_compressPlugin!=0),"-c-?");
    return HDIFF_SUCCESS;
}
*/

/*
    typedef struct hdiff_TCompress{
        //return type tag; strlen(result)<=hpatch_kMaxPluginTypeLength; (Note:result lifetime)
        const char*             (*compressType)(void);//ascii cstring,cannot contain '&'
        //return the max compressed size, if input dataSize data;
        hpatch_StreamPos_t (*maxCompressedSize)(hpatch_StreamPos_t dataSize);
        //return support threadNumber
        int          (*setParallelThreadNumber)(struct hdiff_TCompress* compressPlugin,int threadNum);
        //compress data to out_code; return compressed size, if error or not need compress then return 0;
        //if out_code->write() return hdiff_stream_kCancelCompress(error) then return 0;
        //if memory I/O can use hdiff_compress_mem()
        hpatch_StreamPos_t          (*compress)(const struct hdiff_TCompress* compressPlugin,
                                                const hpatch_TStreamOutput*   out_code,
                                                const hpatch_TStreamInput*    in_data);
        const char*        (*compressTypeForDisplay)(void);//like compressType but just for display,can NULL
    } hdiff_TCompress;
*/

/*
    typedef struct hpatch_TDecompress{
        hpatch_BOOL        (*is_can_open)(const char* compresseType);
        //error return 0.
        hpatch_decompressHandle   (*open)(struct hpatch_TDecompress* decompressPlugin,
                                          hpatch_StreamPos_t dataSize,
                                          const struct hpatch_TStreamInput* codeStream,
                                          hpatch_StreamPos_t code_begin,
                                          hpatch_StreamPos_t code_end);//codeSize==code_end-code_begin
        hpatch_BOOL              (*close)(struct hpatch_TDecompress* decompressPlugin,
                                          hpatch_decompressHandle decompressHandle);
        //decompress_part() must out (out_part_data_end-out_part_data), otherwise error return hpatch_FALSE
        hpatch_BOOL    (*decompress_part)(hpatch_decompressHandle decompressHandle,
                                          unsigned char* out_part_data,unsigned char* out_part_data_end);
        //reset_code add new compressed data; for support vcpatch, can NULL
        hpatch_BOOL         (*reset_code)(hpatch_decompressHandle decompressHandle,
                                          hpatch_StreamPos_t dataSize,
                                          const struct hpatch_TStreamInput* codeStream,
                                          hpatch_StreamPos_t code_begin,
                                          hpatch_StreamPos_t code_end);
        volatile hpatch_dec_error_t decError; //if you used decError value, once patch must used it's own hpatch_TDecompress
    } hpatch_TDecompress;
*/

/*
void create_compressed_diff(const unsigned char* newData,const unsigned char* newData_end,
                            const unsigned char* oldData,const unsigned char* oldData_end,
                            std::vector<unsigned char>& out_diff,
                            const hdiff_TCompress* compressPlugin=0,
                            int kMinSingleMatchScore=kMinSingleMatchScore_default,
                            bool isUseBigCacheMatch=false,
                            ICoverLinesListener* listener=0,size_t threadNum=1);
*/

/*
hpatch_BOOL patch_decompress(const hpatch_TStreamOutput* out_newData,       //sequential write
                             const hpatch_TStreamInput*  oldData,           //random read
                             const hpatch_TStreamInput*  compressedDiff,    //random read
                             hpatch_TDecompress* decompressPlugin);
*/

