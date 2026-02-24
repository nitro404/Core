#include "HDiffPatchDecompressionPluginXz.h"

#include <lzma.h>

/*
#ifdef _CompressPlugin_7zXZ
#if (_IsNeedIncludeDefaultCompressHead)
#   include <SevenZip/C/Xz.h>
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
    
    typedef struct _7zXZ_TDecompress{
        ISzAlloc           memAllocBase;
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        hpatch_TDecompress* decompressPlugin;
        hpatch_BOOL isResetState;
        
        CXzUnpacker     decEnv;
        SizeT           decCopyPos;
        SizeT           decReadPos;
        hpatch_dec_error_t decError;
        unsigned char   dec_buf[kDecompressBufSize];
    } _7zXZ_TDecompress;
    static void * __7zXZ_dec_Alloc(ISzAllocPtr p, size_t size) 
        __dec_Alloc_fun(_7zXZ_TDecompress,p,size)
    
    static hpatch_BOOL _7zXZ_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"7zXZ"));
    }
    static void _7zXZ_open_at(_7zXZ_TDecompress* self, hpatch_TDecompress* decompressPlugin,hpatch_StreamPos_t dataSize,
                              const hpatch_TStreamInput* codeStream,hpatch_StreamPos_t code_begin,
                              hpatch_StreamPos_t code_end,hpatch_BOOL isResetState,hpatch_BOOL isParseHead){
        memset(self,0,sizeof(_7zXZ_TDecompress)-kDecompressBufSize);
        self->memAllocBase.Alloc=__7zXZ_dec_Alloc;
        *((void**)&self->memAllocBase.Free)=(void*)__dec_free;
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->decompressPlugin=decompressPlugin;
        self->isResetState=isResetState;

        self->decCopyPos=0;
        self->decReadPos=kDecompressBufSize;
        
        XzUnpacker_Construct(&self->decEnv,&self->memAllocBase);
        XzUnpacker_Init(&self->decEnv);
        if (!isParseHead)
            self->decEnv.state=XZ_STATE_BLOCK_HEADER;
    }
    static void _7zXZ_close_at(hpatch_decompressHandle decompressHandle){
        _7zXZ_TDecompress* self=(_7zXZ_TDecompress*)decompressHandle;
        if (self){
            hpatch_TDecompress* decompressPlugin=self->decompressPlugin;
            XzUnpacker_Free(&self->decEnv);
            _dec_onDecErr_up();
        }
    }
    static hpatch_decompressHandle _7zXZ_open(hpatch_TDecompress* decompressPlugin,
                                              hpatch_StreamPos_t dataSize,
                                              const hpatch_TStreamInput* codeStream,
                                              hpatch_StreamPos_t code_begin,
                                              hpatch_StreamPos_t code_end){
        _7zXZ_TDecompress* self=0;
        
        self=(_7zXZ_TDecompress*)_dec_malloc(sizeof(_7zXZ_TDecompress));
        if (!self) _dec_memErr_rt();
        _7zXZ_open_at(self,decompressPlugin,dataSize,codeStream,
                      code_begin,code_end,hpatch_FALSE,hpatch_TRUE);
        return self;
    }
    static hpatch_decompressHandle _7zXZ_a_open(hpatch_TDecompress* decompressPlugin,
                                                hpatch_StreamPos_t dataSize,
                                                const hpatch_TStreamInput* codeStream,
                                                hpatch_StreamPos_t code_begin,
                                                hpatch_StreamPos_t code_end){
        _7zXZ_TDecompress* self=0;
        
        self=(_7zXZ_TDecompress*)_dec_malloc(sizeof(_7zXZ_TDecompress));
        if (!self) _dec_memErr_rt();
        _7zXZ_open_at(self,decompressPlugin,dataSize,codeStream,
                      code_begin,code_end,hpatch_TRUE,hpatch_TRUE);
        return self;
    }
    static hpatch_BOOL _7zXZ_close(struct hpatch_TDecompress* decompressPlugin,
                                    hpatch_decompressHandle decompressHandle){
        if (decompressHandle){
            _7zXZ_close_at(decompressHandle);
            free(decompressHandle);
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _7zXZ_reset_code(hpatch_decompressHandle decompressHandle,
                                        hpatch_StreamPos_t dataSize,
                                        const struct hpatch_TStreamInput* codeStream,
                                        hpatch_StreamPos_t code_begin,
                                        hpatch_StreamPos_t code_end){
        _7zXZ_TDecompress* self=(_7zXZ_TDecompress*)decompressHandle;
        hpatch_BOOL isResetState=self->isResetState;
        if (isResetState){
            hpatch_TDecompress* decompressPlugin=self->decompressPlugin;
            _7zXZ_close_at(self);
            _7zXZ_open_at(self,decompressPlugin,dataSize,codeStream,
                          code_begin,code_end,isResetState,hpatch_FALSE);
        }else{
            self->codeStream=codeStream;
            self->code_begin=code_begin;
            self->code_end=code_end;
            self->decCopyPos=0;
            self->decReadPos=kDecompressBufSize;
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _7zXZ_decompress_part(hpatch_decompressHandle decompressHandle,
                                            unsigned char* out_part_data,unsigned char* out_part_data_end){
        _7zXZ_TDecompress* self=(_7zXZ_TDecompress*)decompressHandle;
        unsigned char* out_cur=out_part_data;
        assert(out_part_data<=out_part_data_end);
        while (out_cur<out_part_data_end){
            ECoderStatus status;
            SizeT inSize;
            SizeT outSize=out_part_data_end-out_cur;
            SRes res;
            hpatch_StreamPos_t codeLen=(self->code_end-self->code_begin);
            if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                size_t readLen=kDecompressBufSize;
                if (readLen>codeLen) readLen=(size_t)codeLen;
                self->decReadPos=kDecompressBufSize-readLen;
                if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf+self->decReadPos,
                                            self->dec_buf+self->decReadPos+readLen)) return hpatch_FALSE;//error;
                self->code_begin+=readLen;
                codeLen-=readLen;
            }
            
            inSize=kDecompressBufSize-self->decReadPos;
            res=XzUnpacker_Code(&self->decEnv,out_cur,&outSize,self->dec_buf+self->decReadPos,&inSize,
                                codeLen==0,CODER_FINISH_ANY,&status);
            if(res==SZ_OK){
                if ((inSize==0)&&(outSize==0))
                    _dec_onDecErr_rt();//error;
            }else{
                _dec_onDecErr_rt();//error;
            }
            self->decReadPos+=inSize;
            out_cur+=outSize;
        }
        return hpatch_TRUE;
    }
    static hpatch_TDecompress _7zXZDecompressPlugin={_7zXZ_is_can_open,_7zXZ_open,
                                                      _7zXZ_close,_7zXZ_decompress_part,_7zXZ_reset_code};
    static hpatch_TDecompress _7zXZDecompressPlugin_a={_7zXZ_is_can_open,_7zXZ_a_open,
                                                        _7zXZ_close,_7zXZ_decompress_part,_7zXZ_reset_code};
#endif//_CompressPlugin_7zXZ
*/
