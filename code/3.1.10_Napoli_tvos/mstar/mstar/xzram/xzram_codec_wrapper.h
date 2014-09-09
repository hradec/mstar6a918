#ifndef _XZRAM_CODEC_WRAPPER__H
#define _XZRAM_CODEC_WRAPPER__H

#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

#if (GCC_VERSION == 301)
#ifdef CONFIG_XZRAM_COMPRESS_LZ4
#define XZRAM_COMPRESS_LZ4
#endif
#endif

#ifdef XZRAM_COMPRESS_LZ4
#include <linux/lz4.h>
#else
#include <linux/lzo.h>
#endif

enum  ZRAM_CODEC_ENUM { 
    ZRAM_CODEC_OK,  
    ZRAM_CODEC_ERROR,
    ZRAM_CODEC_OUT_OF_MEMORY,
    ZRAM_CODEC_NOT_COMPRESSIBLE,
    ZRAM_CODEC_INPUT_OVERRUN,
    ZRAM_CODEC_OUTPUT_OVERRUN,
    ZRAM_CODEC_LOOKBEHIND_OVERRUN,
    ZRAM_CODEC_EOF_NOT_FOUND,
    ZRAM_CODEC_INPUT_NOT_CONSUMED,
    ZRAM_CODEC_NOT_YET_IMPLEMENTED,
    ZRAM_CODEC_INVALID_ARGUMENT,
    ZRAM_CODEC_ERROR_CHECKSUM
};

int compress_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len,  void* work_buf);
int decompres_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len);
u64 gettime(void);

#ifdef XZRAM_COMPRESS_LZ4
#define ZRAM_CODEC_MEM (1<<MEMORY_USAGE)
#else
#define ZRAM_CODEC_MEM LZO1X_MEM_COMPRESS
#endif
#endif
