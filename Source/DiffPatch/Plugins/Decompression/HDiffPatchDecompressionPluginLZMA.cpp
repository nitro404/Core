#include "HDiffPatchDecompressionPluginLZMA.h"

#include <lzma.h>

/*
#if (defined _CompressPlugin_lzma) || (defined _CompressPlugin_lzma2)
#if (_IsNeedIncludeDefaultCompressHead)
#   include <SevenZip/C/LzmaDec.h>
#   ifdef _CompressPlugin_lzma2
#       include <SevenZip/C/Lzma2Dec.h>
#   endif
#endif
#endif

#ifdef _CompressPlugin_lzma
    typedef struct _lzma_TDecompress{
        ISzAlloc           memAllocBase;
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        CLzmaDec        decEnv;
        SizeT           decCopyPos;
        SizeT           decReadPos;
        hpatch_dec_error_t decError;
        unsigned char   dec_buf[kDecompressBufSize];
    } _lzma_TDecompress;
    static void * __lzma1_dec_Alloc(ISzAllocPtr p, size_t size) 
        __dec_Alloc_fun(_lzma_TDecompress,p,size)

    static hpatch_BOOL _lzma_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"lzma"));
    }
    static hpatch_decompressHandle  _lzma_open(hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _lzma_TDecompress* self=0;
        SRes ret;
        unsigned char propsSize=0;
        unsigned char props[256];
        //load propsSize
        if (code_end-code_begin<1) _dec_openErr_rt();
        if (!codeStream->read(codeStream,code_begin,&propsSize,&propsSize+1)) return 0;
        ++code_begin;
        if (propsSize>(code_end-code_begin)) _dec_openErr_rt();
        //load props
        if (!codeStream->read(codeStream,code_begin,props,props+propsSize)) return 0;
        code_begin+=propsSize;

        self=(_lzma_TDecompress*)_dec_malloc(sizeof(_lzma_TDecompress));
        if (!self) _dec_memErr_rt();
        memset(self,0,sizeof(_lzma_TDecompress)-kDecompressBufSize);
        self->memAllocBase.Alloc=__lzma1_dec_Alloc;
        *((void**)&self->memAllocBase.Free)=(void*)__dec_free;
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        self->decCopyPos=0;
        self->decReadPos=kDecompressBufSize;
        
        LzmaDec_Construct(&self->decEnv);
        ret=LzmaDec_Allocate(&self->decEnv,props,propsSize,&self->memAllocBase);
        if (ret!=SZ_OK){ _dec_onDecErr_up(); free(self); _dec_openErr_rt(); }
        LzmaDec_Init(&self->decEnv);
        return self;
    }
    static hpatch_BOOL _lzma_close(struct hpatch_TDecompress* decompressPlugin,
                                   hpatch_decompressHandle decompressHandle){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        if (!self) return hpatch_TRUE;
        LzmaDec_Free(&self->decEnv,&self->memAllocBase);
        _dec_onDecErr_up();
        free(self);
        return hpatch_TRUE;
    }
    static hpatch_BOOL _lzma_decompress_part(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        unsigned char* out_cur=out_part_data;
        assert(out_part_data<=out_part_data_end);
        while (out_cur<out_part_data_end){
            size_t copyLen=(self->decEnv.dicPos-self->decCopyPos);
            if (copyLen>0){
                if (copyLen>(size_t)(out_part_data_end-out_cur))
                    copyLen=(out_part_data_end-out_cur);
                memcpy(out_cur,self->decEnv.dic+self->decCopyPos,copyLen);
                out_cur+=copyLen;
                self->decCopyPos+=copyLen;
                if ((self->decEnv.dicPos==self->decEnv.dicBufSize)
                    &&(self->decEnv.dicPos==self->decCopyPos)){
                    self->decEnv.dicPos=0;
                    self->decCopyPos=0;
                }
            }else{
                ELzmaStatus status;
                SizeT inSize,dicPos_back;
                SRes res;
                hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
                if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                    size_t readLen=kDecompressBufSize;
                    if (readLen>codeLen) readLen=(size_t)codeLen;
                    self->decReadPos=kDecompressBufSize-readLen;
                    if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf+self->decReadPos,
                                                self->dec_buf+self->decReadPos+readLen)) return hpatch_FALSE;//error;
                    self->code_begin+=readLen;
                }

                inSize=kDecompressBufSize-self->decReadPos;
                dicPos_back=self->decEnv.dicPos;
                res=LzmaDec_DecodeToDic(&self->decEnv,self->decEnv.dicBufSize,
                                        self->dec_buf+self->decReadPos,&inSize,LZMA_FINISH_ANY,&status);
                if(res==SZ_OK){
                    if ((inSize==0)&&(self->decEnv.dicPos==dicPos_back))
                        _dec_onDecErr_rt();//error;
                }else{
                    _dec_onDecErr_rt();//error;
                }
                self->decReadPos+=inSize;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress lzmaDecompressPlugin={_lzma_is_can_open,_lzma_open,
                                                    _lzma_close,_lzma_decompress_part};
#endif//_CompressPlugin_lzma

#ifdef _CompressPlugin_lzma2
    typedef struct _lzma2_TDecompress{
        ISzAlloc           memAllocBase;
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        CLzma2Dec       decEnv;
        SizeT           decCopyPos;
        SizeT           decReadPos;
        hpatch_dec_error_t decError;
        unsigned char   dec_buf[kDecompressBufSize];
    } _lzma2_TDecompress;
    static void * __lzma2_dec_Alloc(ISzAllocPtr p, size_t size) 
        __dec_Alloc_fun(_lzma2_TDecompress,p,size)
    
    static hpatch_BOOL _lzma2_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"lzma2"));
    }
    static hpatch_decompressHandle _lzma2_open(hpatch_TDecompress* decompressPlugin,
                                            hpatch_StreamPos_t dataSize,
                                            const hpatch_TStreamInput* codeStream,
                                            hpatch_StreamPos_t code_begin,
                                            hpatch_StreamPos_t code_end){
        _lzma2_TDecompress* self=0;
        SRes ret;
        unsigned char propsSize=0;
        //load propsSize
        if (code_end-code_begin<1) _dec_openErr_rt();
        if (!codeStream->read(codeStream,code_begin,&propsSize,&propsSize+1)) return 0;
        ++code_begin;
        
        self=(_lzma2_TDecompress*)_dec_malloc(sizeof(_lzma2_TDecompress));
        if (!self) _dec_memErr_rt();
        memset(self,0,sizeof(_lzma2_TDecompress)-kDecompressBufSize);
        self->memAllocBase.Alloc=__lzma2_dec_Alloc;
        *((void**)&self->memAllocBase.Free)=(void*)__dec_free;
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        self->decCopyPos=0;
        self->decReadPos=kDecompressBufSize;
        
        Lzma2Dec_Construct(&self->decEnv);
        ret=Lzma2Dec_Allocate(&self->decEnv,propsSize,&self->memAllocBase);
        if (ret!=SZ_OK){ _dec_onDecErr_up(); free(self); _dec_openErr_rt(); }
        Lzma2Dec_Init(&self->decEnv);
        return self;
    }
    static hpatch_BOOL _lzma2_close(struct hpatch_TDecompress* decompressPlugin,
                                    hpatch_decompressHandle decompressHandle){
        _lzma2_TDecompress* self=(_lzma2_TDecompress*)decompressHandle;
        if (!self) return hpatch_TRUE;
        Lzma2Dec_Free(&self->decEnv,&self->memAllocBase);
        _dec_onDecErr_up();
        free(self);
        return hpatch_TRUE;
    }
    static hpatch_BOOL _lzma2_decompress_part(hpatch_decompressHandle decompressHandle,
                                            unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lzma2_TDecompress* self=(_lzma2_TDecompress*)decompressHandle;
        unsigned char* out_cur=out_part_data;
        assert(out_part_data<=out_part_data_end);
        while (out_cur<out_part_data_end){
            size_t copyLen=(self->decEnv.decoder.dicPos-self->decCopyPos);
            if (copyLen>0){
                if (copyLen>(size_t)(out_part_data_end-out_cur))
                    copyLen=(out_part_data_end-out_cur);
                memcpy(out_cur,self->decEnv.decoder.dic+self->decCopyPos,copyLen);
                out_cur+=copyLen;
                self->decCopyPos+=copyLen;
                if ((self->decEnv.decoder.dicPos==self->decEnv.decoder.dicBufSize)
                    &&(self->decEnv.decoder.dicPos==self->decCopyPos)){
                    self->decEnv.decoder.dicPos=0;
                    self->decCopyPos=0;
                }
            }else{
                ELzmaStatus status;
                SizeT inSize,dicPos_back;
                SRes res;
                hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
                if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                    size_t readLen=kDecompressBufSize;
                    if (readLen>codeLen) readLen=(size_t)codeLen;
                    self->decReadPos=kDecompressBufSize-readLen;
                    if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf+self->decReadPos,
                                                self->dec_buf+self->decReadPos+readLen)) return hpatch_FALSE;//error;
                    self->code_begin+=readLen;
                }
                
                inSize=kDecompressBufSize-self->decReadPos;
                dicPos_back=self->decEnv.decoder.dicPos;
                res=Lzma2Dec_DecodeToDic(&self->decEnv,self->decEnv.decoder.dicBufSize,
                                        self->dec_buf+self->decReadPos,&inSize,LZMA_FINISH_ANY,&status);
                if(res==SZ_OK){
                    if ((inSize==0)&&(self->decEnv.decoder.dicPos==dicPos_back))
                        _dec_onDecErr_rt();//error;
                }else{
                    _dec_onDecErr_rt();//error;
                }
                self->decReadPos+=inSize;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress lzma2DecompressPlugin={_lzma2_is_can_open,_lzma2_open,
                                                    _lzma2_close,_lzma2_decompress_part};
#endif//_CompressPlugin_lzma2
*/
