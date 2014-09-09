#ifndef _ZRAM_CODEC_WRAPPER__H
#define _ZRAM_CODEC_WRAPPER__H

#include <linux/lzo.h>
#include "zram_codec_common.h"

//#define USE_LZ4
#define USE_LZO
//#define USE_BYPASS

//default enable on dev. branch, only enable this on main trunk when needed.
//#define ZRAM_STAT

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
u32 gettime(void);

#ifdef USE_LZ4
#define ZRAM_CODEC_MEM (1<<MEMORY_USAGE)
#else
#define ZRAM_CODEC_MEM LZO1X_MEM_COMPRESS
#endif
#endif
