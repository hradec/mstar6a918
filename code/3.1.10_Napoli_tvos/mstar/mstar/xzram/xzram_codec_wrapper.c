#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/time.h>    
#include "xzram_codec_wrapper.h"

#ifdef  ZRAM_CHECKSUM
#define CHECKSUM_HEADER_LEN 8
#define CHECKSUM_ID_POS 0
#define CHECKSUM_PAYLOAD_LEN_POS 4
#define CHECKSUM_PAYLOAD_CRC_POS 6
static int zram_counter;
static unsigned short crc_checksum(const char* buf, int buf_len);
#endif

#ifdef CONFIG_XZRAM_COMPRESS_PERFORMANCE_STAT
__always_inline u64 gettime(void) 
{
	struct timeval time;
	do_gettimeofday(&time);
	return (time.tv_sec*1000000 + time.tv_usec);
}
#endif

#ifdef XZRAM_COMPRESS_LZ4
int compress_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len,  void* work_buf)
{
	#ifdef ZRAM_CHECKSUM 
	*dst_len = LZ4_compress(src_buf, dst_buf+CHECKSUM_HEADER_LEN, work_buf, src_len);
			 
	//8 bytes header : 4 byte serial id, 2 byte dst len, 2 byte crc checksum
	zram_counter ++;
	*((unsigned int*)dst_buf) = zram_counter;	  
	*((unsigned short*)(dst_buf+CHECKSUM_PAYLOAD_LEN_POS)) = (unsigned short)(*dst_len);	 
	*((unsigned short*)(dst_buf+CHECKSUM_PAYLOAD_CRC_POS)) = crc_checksum(dst_buf+CHECKSUM_HEADER_LEN,*dst_len);	 
	*dst_len += CHECKSUM_HEADER_LEN;			 
	#else
	*dst_len = LZ4_compress(src_buf, dst_buf, work_buf, src_len);
	#endif
	return ZRAM_CODEC_OK;
}

int decompres_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len)
{  
	#ifdef ZRAM_CHECKSUM
	unsigned short header_crc, compute_crc;
	header_crc = *((unsigned short*)(src_buf+CHECKSUM_PAYLOAD_CRC_POS));
	compute_crc = crc_checksum(src_buf+CHECKSUM_HEADER_LEN,*((unsigned short*)(src_buf+CHECKSUM_PAYLOAD_LEN_POS)));
		
	if(header_crc == compute_crc)
	{
	*dst_len = LZ4_uncompress_unknownOutputSize(src_buf+CHECKSUM_HEADER_LEN, dst_buf, src_len-CHECKSUM_HEADER_LEN, *dst_len);			
	}
	else
	{
	   ret = ZRAM_CODEC_ERROR_CHECKSUM;
	}			 
	#else
	*dst_len = LZ4_uncompress_unknownOutputSize(src_buf, dst_buf, src_len, *dst_len);
	#endif
	return ZRAM_CODEC_OK;
}

#else
int compress_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len, void* work_buf)
{
    int ret, compress_ret;

	#ifdef ZRAM_CHECKSUM	
	compress_ret = lzo1x_1_compress(src_buf,src_len,dst_buf+CHECKSUM_HEADER_LEN,dst_len,work_buf);

	//8 bytes header : 4 byte serial id, 2 byte dst len, 2 byte crc checksum
	zram_counter ++;
	*((unsigned int*)(dst_buf+CHECKSUM_ID_POS)) = zram_counter;
	*((unsigned short*)(dst_buf+CHECKSUM_PAYLOAD_LEN_POS)) = (unsigned short)(*dst_len);	
	*((unsigned short*)(dst_buf+CHECKSUM_PAYLOAD_CRC_POS)) = crc_checksum(dst_buf+CHECKSUM_HEADER_LEN,*dst_len);	
	*dst_len += CHECKSUM_HEADER_LEN;
	#else
	compress_ret = lzo1x_1_compress(src_buf,src_len,dst_buf,dst_len,work_buf);
	#endif

	if(compress_ret == LZO_E_OK)
	    ret = ZRAM_CODEC_OK;
	else
	    ret = ZRAM_CODEC_ERROR;

	return ret;
}

int decompres_func(const char* src_buf, int src_len,char* dst_buf, int* dst_len)
{
	int ret = ZRAM_CODEC_OK; 
	int decompress_ret;

	#ifdef ZRAM_CHECKSUM
	unsigned short header_crc, compute_crc;
	header_crc = *((unsigned short*)(src_buf+CHECKSUM_PAYLOAD_CRC_POS));
	compute_crc = crc_checksum(src_buf+CHECKSUM_HEADER_LEN,*((unsigned short*)(src_buf+CHECKSUM_PAYLOAD_LEN_POS)));
			
	if(header_crc == compute_crc)
	{			 
		decompress_ret = lzo1x_decompress_safe(src_buf+CHECKSUM_HEADER_LEN,src_len-CHECKSUM_HEADER_LEN,dst_buf,dst_len);		
	}
	else
	{
	    ret = ZRAM_CODEC_ERROR_CHECKSUM;
	}	 
	#else 	
	decompress_ret = lzo1x_decompress_safe(src_buf,src_len,dst_buf,dst_len);
	#endif

	if(decompress_ret == LZO_E_OK)
	    ret = ZRAM_CODEC_OK;
	else if (ret != ZRAM_CODEC_ERROR_CHECKSUM)
	    ret = ZRAM_CODEC_ERROR;

	return ret;
}
#endif

#ifdef ZRAM_CHECKSUM
static int get_bit_pos(int val, int pos)
{
	return (val >> pos) & 0x1;
}
static unsigned short crc32(const unsigned int din, const unsigned short c)
{
    int				i, crc_out;
    unsigned int	d = din;
    unsigned char	newcrc[16] = {0};

    newcrc[0]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 30) ^ get_bit_pos(d, 27) ^ get_bit_pos(d, 26) ^ get_bit_pos(d, 25) ^ get_bit_pos(d, 24) ^ get_bit_pos(d, 23) ^ get_bit_pos(d, 22) ^ get_bit_pos(d, 21) ^ get_bit_pos(d, 20) ^ get_bit_pos(d, 19) ^ get_bit_pos(d, 18) ^ get_bit_pos(d, 17) ^ get_bit_pos(d, 16) ^ get_bit_pos(d, 15) ^ get_bit_pos(d, 13) ^ get_bit_pos(d, 12) ^ get_bit_pos(d, 11) ^ get_bit_pos(d, 10) ^ get_bit_pos(d, 9) ^ get_bit_pos(d, 8) ^ get_bit_pos(d, 7) ^ get_bit_pos(d, 6) ^ get_bit_pos(d, 5) ^ get_bit_pos(d, 4) ^ get_bit_pos(d, 3) ^ get_bit_pos(d, 2) ^ get_bit_pos(d, 1) ^ get_bit_pos(d, 0) ^ get_bit_pos(c, 0) ^ get_bit_pos(c, 1) ^ get_bit_pos(c, 2) ^ get_bit_pos(c, 3) ^ get_bit_pos(c, 4) ^ get_bit_pos(c, 5) ^ get_bit_pos(c, 6) ^ get_bit_pos(c, 7) ^ get_bit_pos(c, 8) ^ get_bit_pos(c, 9) ^ get_bit_pos(c, 10) ^ get_bit_pos(c, 11) ^ get_bit_pos(c, 14) ^ get_bit_pos(c, 15);
    newcrc[1]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 28) ^ get_bit_pos(d, 27) ^ get_bit_pos(d, 26) ^ get_bit_pos(d, 25) ^ get_bit_pos(d, 24) ^ get_bit_pos(d, 23) ^ get_bit_pos(d, 22) ^ get_bit_pos(d, 21) ^ get_bit_pos(d, 20) ^ get_bit_pos(d, 19) ^ get_bit_pos(d, 18) ^ get_bit_pos(d, 17) ^ get_bit_pos(d, 16) ^ get_bit_pos(d, 14) ^ get_bit_pos(d, 13) ^ get_bit_pos(d, 12) ^ get_bit_pos(d, 11) ^ get_bit_pos(d, 10) ^ get_bit_pos(d, 9) ^ get_bit_pos(d, 8) ^ get_bit_pos(d, 7) ^ get_bit_pos(d, 6) ^ get_bit_pos(d, 5) ^ get_bit_pos(d, 4) ^ get_bit_pos(d, 3) ^ get_bit_pos(d, 2) ^ get_bit_pos(d, 1) ^ get_bit_pos(c, 0) ^ get_bit_pos(c, 1) ^ get_bit_pos(c, 2) ^ get_bit_pos(c, 3) ^ get_bit_pos(c, 4) ^ get_bit_pos(c, 5) ^ get_bit_pos(c, 6) ^ get_bit_pos(c, 7) ^ get_bit_pos(c, 8) ^ get_bit_pos(c, 9) ^ get_bit_pos(c, 10) ^ get_bit_pos(c, 11) ^ get_bit_pos(c, 12) ^ get_bit_pos(c, 15);
    newcrc[2]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 30) ^ get_bit_pos(d, 29) ^ get_bit_pos(d, 28) ^ get_bit_pos(d, 16) ^ get_bit_pos(d, 14) ^ get_bit_pos(d, 1) ^ get_bit_pos(d, 0) ^ get_bit_pos(c, 0) ^ get_bit_pos(c, 12) ^ get_bit_pos(c, 13) ^ get_bit_pos(c, 14) ^ get_bit_pos(c, 15);
    newcrc[3]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 30) ^ get_bit_pos(d, 29) ^ get_bit_pos(d, 17) ^ get_bit_pos(d, 15) ^ get_bit_pos(d, 2) ^ get_bit_pos(d, 1) ^ get_bit_pos(c, 1) ^ get_bit_pos(c, 13) ^ get_bit_pos(c, 14) ^ get_bit_pos(c, 15);
    newcrc[4]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 30) ^ get_bit_pos(d, 18) ^ get_bit_pos(d, 16) ^ get_bit_pos(d, 3) ^ get_bit_pos(d, 2) ^ get_bit_pos(c, 0) ^ get_bit_pos(c, 2) ^ get_bit_pos(c, 14) ^ get_bit_pos(c, 15);
    newcrc[5]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 19) ^ get_bit_pos(d, 17) ^ get_bit_pos(d, 4) ^ get_bit_pos(d, 3) ^ get_bit_pos(c, 1) ^ get_bit_pos(c, 3) ^ get_bit_pos(c, 15);
    newcrc[6]	= get_bit_pos(d, 20) ^ get_bit_pos(d, 18) ^ get_bit_pos(d, 5) ^ get_bit_pos(d, 4) ^ get_bit_pos(c, 2) ^ get_bit_pos(c, 4);
    newcrc[7]	= get_bit_pos(d, 21) ^ get_bit_pos(d, 19) ^ get_bit_pos(d, 6) ^ get_bit_pos(d, 5) ^ get_bit_pos(c, 3) ^ get_bit_pos(c, 5);
    newcrc[8]	= get_bit_pos(d, 22) ^ get_bit_pos(d, 20) ^ get_bit_pos(d, 7) ^ get_bit_pos(d, 6) ^ get_bit_pos(c, 4) ^ get_bit_pos(c, 6);
    newcrc[9]	= get_bit_pos(d, 23) ^ get_bit_pos(d, 21) ^ get_bit_pos(d, 8) ^ get_bit_pos(d, 7) ^ get_bit_pos(c, 5) ^ get_bit_pos(c, 7);
    newcrc[10]	= get_bit_pos(d, 24) ^ get_bit_pos(d, 22) ^ get_bit_pos(d, 9) ^ get_bit_pos(d, 8) ^ get_bit_pos(c, 6) ^ get_bit_pos(c, 8);
    newcrc[11]	= get_bit_pos(d, 25) ^ get_bit_pos(d, 23) ^ get_bit_pos(d, 10) ^ get_bit_pos(d, 9) ^ get_bit_pos(c, 7) ^ get_bit_pos(c, 9);
    newcrc[12]	= get_bit_pos(d, 26) ^ get_bit_pos(d, 24) ^ get_bit_pos(d, 11) ^ get_bit_pos(d, 10) ^ get_bit_pos(c, 8) ^ get_bit_pos(c, 10);
    newcrc[13]	= get_bit_pos(d, 27) ^ get_bit_pos(d, 25) ^ get_bit_pos(d, 12) ^ get_bit_pos(d, 11) ^ get_bit_pos(c, 9) ^ get_bit_pos(c, 11);
    newcrc[14]	= get_bit_pos(d, 28) ^ get_bit_pos(d, 26) ^ get_bit_pos(d, 13) ^ get_bit_pos(d, 12) ^ get_bit_pos(c, 10) ^ get_bit_pos(c, 12);
    newcrc[15]	= get_bit_pos(d, 31) ^ get_bit_pos(d, 30) ^ get_bit_pos(d, 29) ^ get_bit_pos(d, 26) ^ get_bit_pos(d, 25) ^ get_bit_pos(d, 24) ^ get_bit_pos(d, 23) ^ get_bit_pos(d, 22) ^ get_bit_pos(d, 21) ^ get_bit_pos(d, 20) ^ get_bit_pos(d, 19) ^ get_bit_pos(d, 18) ^ get_bit_pos(d, 17) ^ get_bit_pos(d, 16) ^ get_bit_pos(d, 15) ^ get_bit_pos(d, 14) ^ get_bit_pos(d, 12) ^ get_bit_pos(d, 11) ^ get_bit_pos(d, 10) ^ get_bit_pos(d, 9) ^ get_bit_pos(d, 8) ^ get_bit_pos(d, 7) ^ get_bit_pos(d, 6) ^ get_bit_pos(d, 5) ^ get_bit_pos(d, 4) ^ get_bit_pos(d, 3) ^ get_bit_pos(d, 2) ^ get_bit_pos(d, 1) ^ get_bit_pos(d, 0) ^ get_bit_pos(c, 0) ^ get_bit_pos(c, 1) ^ get_bit_pos(c, 2) ^ get_bit_pos(c, 3) ^ get_bit_pos(c, 4) ^ get_bit_pos(c, 5) ^ get_bit_pos(c, 6) ^ get_bit_pos(c, 7) ^ get_bit_pos(c, 8) ^ get_bit_pos(c, 9) ^ get_bit_pos(c, 10) ^ get_bit_pos(c, 13) ^ get_bit_pos(c, 14) ^ get_bit_pos(c, 15);

    crc_out = 0;
    for (i = 0; i < 16; i++)
        crc_out += (newcrc[i] << i);

    return crc_out;
}

static unsigned short crc_checksum(const char* buf, int buf_len)
{
	int i;
	unsigned short crc = 0;
	
	for(i=0;i<buf_len;i++)  crc = crc32( buf[i], crc);
	
	return crc;
}
#endif
