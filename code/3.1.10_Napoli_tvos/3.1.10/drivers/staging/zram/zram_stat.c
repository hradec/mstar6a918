#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#define procfs_name "zramstat"

struct proc_dir_entry *proc_file;
extern struct proc_dir_entry proc_root;
u32 cnt_rd = 0;
u32 cnt_wr = 0;
u32 cnt_enc = 0;
u32 cnt_dec = 0;
u32 cnt_drop = 0;
u32 time_rd = 0;
u32 time_wr = 0;
u32 time_enc = 0;
u32 time_dec = 0;

u64 compr_size = 0;		/* compressed size of pages stored */
u64 num_reads = 0;		/* failed + successful */
u64 num_writes = 0;		/* --do-- */
u64 failed_reads = 0;	/* should NEVER! happen */
u64 failed_writes = 0;	/* can happen when memory is too low */
u64 invalid_io = 0;		/* non-page-aligned I/O requests */
u64 notify_free = 0;	/* no. of swap slot free notifications */
u32 pages_zero = 0;		/* no. of zero filled pages */
u32 pages_stored = 0;	/* no. of pages currently stored */
u32 good_compress = 0;	/* % of pages with compression ratio<=50% */
u32 pages_expand = 0;	/* % of incompressible pages */
u32 kswapd_pgout = 0;	/* no. of kswapd pgout */
u32 kswapd_comprsize = 0;	/* no. of kswapd compress size */

int zram_procfile_read(char *buffer,
        char **buffer_location,
        off_t offset, int buffer_length, int *eof, void *data)
{
    int ret;

    if (offset > 0) {
        ret  = 0;
    } else {
        //ret = sprintf(buffer, "HelloWorld!\n");
        ret = sprintf(buffer, "write count:\t%u\n"
                              "read count:\t%u\n"
                              "encode count:\t%u\n"
                              "decode count:\t%u\n"
                              "drop count:\t%u\n"
                              "write time:\t%u ns\n"
                              "read time:\t%u ns\n"
                              "encode time:\t%u ns\n"
                              "decode time:\t%u ns\n"
                              "\n\n"
                              "zram stat\n"
                              "compr_size:\t%llu\n"
                              "num_reads:\t%llu\n"
                              "num_writes:\t%llu\n"
                              "failed_reads:\t%llu\n"
                              "failed_writes:\t%llu\n"
                              "invalid_io:\t%llu\n"
                              "notify_free:\t%llu\n"
                              "pages_zero:\t%u\n"
                              "pages_stored:\t%u\n"
                              "good_compress:\t%u\n"
                              "pages_expand:\t%u\n"
                              "kswapd_pgout:\t%u\n"
                              "kswapd_comprsize:\t%u\n",
                               cnt_wr, cnt_rd, cnt_enc, cnt_dec, cnt_drop, time_wr, time_rd, time_enc, time_dec,
                               compr_size, num_reads, num_writes, failed_reads, failed_writes, invalid_io, notify_free, pages_zero, pages_stored, good_compress, pages_expand, kswapd_pgout, kswapd_comprsize);

    }

    return ret;
}

int init_module()
{
    proc_file = create_proc_entry(procfs_name, 0644, NULL);

    if (proc_file == NULL) {
        remove_proc_entry(procfs_name, &proc_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
        return -ENOMEM;
    }

    proc_file->read_proc = zram_procfile_read;
    proc_file->mode  = S_IFREG | S_IRUGO;
    proc_file->uid   = 0;
    proc_file->gid   = 0;
    proc_file->size  = 37;

    printk(KERN_INFO "/proc/%s created\n", procfs_name);
    return 0;
}

void cleanup_module()
{
    remove_proc_entry(procfs_name, &proc_root);
    printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}

module_init(init_module);
module_exit(cleanup_module);
