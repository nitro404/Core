#include "HDiffPatchDecompressionPluginBZip2.h"

#include <bzlib.h>

/*
#ifdef  _CompressPlugin_bz2
#if (_IsNeedIncludeDefaultCompressHead)
#   include "bzlib.h" // http://www.bzip.org/  https://github.com/sisong/bzip2
#endif
    typedef struct _bz2_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        bz_stream       d_stream;
        hpatch_dec_error_t decError;
        unsigned char   dec_buf[kDecompressBufSize];
    } _bz2_TDecompress;
    static hpatch_BOOL _bz2_is_can_open(const char* compressType){
        return (0==strcmp(compressType,"bz2"))||(0==strcmp(compressType,"bzip2"))
             ||(0==strcmp(compressType,"pbz2"))||(0==strcmp(compressType,"pbzip2"));
    }
    static hpatch_decompressHandle  _bz2_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        int ret;
        _bz2_TDecompress* self=(_bz2_TDecompress*)_dec_malloc(sizeof(_bz2_TDecompress));
        if (!self) _dec_memErr_rt();
        memset(self,0,sizeof(_bz2_TDecompress)-kDecompressBufSize);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        ret=BZ2_bzDecompressInit(&self->d_stream,0,0);
        if (ret!=BZ_OK){ free(self); _dec_openErr_rt(); }
        return self;
    }
    static hpatch_BOOL _bz2_close(struct hpatch_TDecompress* decompressPlugin,
                                  hpatch_decompressHandle decompressHandle){
        hpatch_BOOL result=hpatch_TRUE;
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        if (!self) return result;
        _dec_onDecErr_up();
        _dec_close_check(BZ_OK==BZ2_bzDecompressEnd(&self->d_stream));
        free(self);
        return result;
    }
    static hpatch_BOOL _bz2_reset_for_next_node(_bz2_TDecompress* self){
        //backup
        char*   next_out_back=self->d_stream.next_out;
        char*   next_in_back=self->d_stream.next_in;
        unsigned int avail_out_back=self->d_stream.avail_out;
        unsigned int avail_in_back=self->d_stream.avail_in;
        //reset
        if (BZ_OK!=BZ2_bzDecompressEnd(&self->d_stream)) _dec_onDecErr_rt();
        if (BZ_OK!=BZ2_bzDecompressInit(&self->d_stream,0,0)) _dec_onDecErr_rt();
        //restore
        self->d_stream.next_out=next_out_back;
        self->d_stream.next_in=next_in_back;
        self->d_stream.avail_out=avail_out_back;
        self->d_stream.avail_in=avail_in_back;
        return hpatch_TRUE;
    }

    static hpatch_BOOL _bz2_decompress_part_(hpatch_decompressHandle decompressHandle,
                                             unsigned char* out_part_data,unsigned char* out_part_data_end,
                                             hpatch_BOOL isMustOutData){
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        assert(out_part_data<=out_part_data_end);
        
        self->d_stream.next_out =(char*)out_part_data;
        self->d_stream.avail_out =(unsigned int)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            unsigned int avail_out_back,avail_in_back;
            int ret;
            hpatch_StreamPos_t codeLen=(self->code_end - self->code_begin);
            if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
                size_t readLen=kDecompressBufSize;
                self->d_stream.next_in=(char*)self->dec_buf;
                if (readLen>codeLen) readLen=(size_t)codeLen;
                if (!self->codeStream->read(self->codeStream,self->code_begin,self->dec_buf,
                                            self->dec_buf+readLen)) return hpatch_FALSE;//error;
                self->d_stream.avail_in=(unsigned int)readLen;
                self->code_begin+=readLen;
                codeLen-=readLen;
            }
            
            avail_out_back=self->d_stream.avail_out;
            avail_in_back=self->d_stream.avail_in;
            ret=BZ2_bzDecompress(&self->d_stream);
            if (ret==BZ_OK){
                if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                    _dec_onDecErr_rt();//error;
            }else if (ret==BZ_STREAM_END){
                if (self->d_stream.avail_in+codeLen>0){ //next compress node!
                    if (!_bz2_reset_for_next_node(self))
                        return hpatch_FALSE;//error;
                }else{//all end
                    if (self->d_stream.avail_out!=0){
                        if (isMustOutData){ //fill out 0
                            memset(self->d_stream.next_out,0,self->d_stream.avail_out);
                            self->d_stream.next_out+=self->d_stream.avail_out;
                            self->d_stream.avail_out=0;
                        }else{
                            _dec_onDecErr_rt();//error;
                        }
                    }
                }
            }else{
                _dec_onDecErr_rt();//error;
            }
        }
        return hpatch_TRUE;
    }
    static hpatch_BOOL _bz2_decompress_part(hpatch_decompressHandle decompressHandle,
                                            unsigned char* out_part_data,unsigned char* out_part_data_end){
        return _bz2_decompress_part_(decompressHandle,out_part_data,out_part_data_end,hpatch_FALSE);
    }
    static hpatch_BOOL _bz2_decompress_part_unsz(hpatch_decompressHandle decompressHandle,
                                                 unsigned char* out_part_data,unsigned char* out_part_data_end){
        return _bz2_decompress_part_(decompressHandle,out_part_data,out_part_data_end,hpatch_TRUE);
    }
    
    static hpatch_TDecompress bz2DecompressPlugin={_bz2_is_can_open,_bz2_open,
                                                   _bz2_close,_bz2_decompress_part};

    //unkown uncompress data size
    static hpatch_TDecompress _bz2DecompressPlugin_unsz={_bz2_is_can_open,_bz2_open,
                                                         _bz2_close,_bz2_decompress_part_unsz};
#endif//_CompressPlugin_bz2
*/
