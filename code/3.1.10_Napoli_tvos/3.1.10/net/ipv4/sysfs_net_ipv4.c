/*
 * net/ipv4/sysfs_net_ipv4.c
 *
 * sysfs-based networking knobs (so we can, unlike with sysctl, control perms)
 *
 * Copyright (C) 2008 Google, Inc.
 *
 * Robert Love <rlove@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <net/tcp.h>

#define CREATE_IPV4_FILE(_name, _var) \
static ssize_t _name##_show(struct kobject *kobj, \
			    struct kobj_attribute *attr, char *buf) \
{ \
	return sprintf(buf, "%d\n", _var); \
} \
static ssize_t _name##_store(struct kobject *kobj, \
			     struct kobj_attribute *attr, \
			     const char *buf, size_t count) \
{ \
	int val, ret; \
	ret = sscanf(buf, "%d", &val); \
	if (ret != 1) \
		return -EINVAL; \
	if (val < 0) \
		return -EINVAL; \
	_var = val; \
	return count; \
} \
static struct kobj_attribute _name##_attr = \
	__ATTR(_name, 0644, _name##_show, _name##_store)

CREATE_IPV4_FILE(tcp_wmem_min, sysctl_tcp_wmem[0]);
CREATE_IPV4_FILE(tcp_wmem_def, sysctl_tcp_wmem[1]);
CREATE_IPV4_FILE(tcp_wmem_max, sysctl_tcp_wmem[2]);

CREATE_IPV4_FILE(tcp_rmem_min, sysctl_tcp_rmem[0]);
CREATE_IPV4_FILE(tcp_rmem_def, sysctl_tcp_rmem[1]);
CREATE_IPV4_FILE(tcp_rmem_max, sysctl_tcp_rmem[2]);

static struct attribute *ipv4_attrs[] = {
	&tcp_wmem_min_attr.attr,
	&tcp_wmem_def_attr.attr,
	&tcp_wmem_max_attr.attr,
	&tcp_rmem_min_attr.attr,
	&tcp_rmem_def_attr.attr,
	&tcp_rmem_max_attr.attr,
	NULL
};

static struct attribute_group ipv4_attr_group = {
	.attrs = ipv4_attrs,
};


#ifdef CONFIG_MP_DEBUG_TOOL_RTP_TRACE

unsigned int rtp_src_port=0;
unsigned int rtp_trace_flag=0;
unsigned int rtp_trace_cnt=0;

static ssize_t rtp_port_show(struct kobject *kobj, 
                   struct kobj_attribute *attr, char *buf)
{
       return printk("RTP Tracking: Port Listening = %d\n", rtp_src_port);
}

static ssize_t rtp_port_store(struct kobject *kobj, 
                             struct kobj_attribute *attr, 
                             const char *buf, size_t count)
{
        int ret; 
        u64 port_num;
        ret = (u32)strict_strtoull(buf, 10, &port_num);

        rtp_src_port = (u32)port_num; 

        return count; 
}

static ssize_t rtp_flag_show(struct kobject *kobj, 
                   struct kobj_attribute *attr, char *buf)
{
       return printk("RTP Tracking: Trace Flag = %d\n", rtp_trace_flag);
}

static ssize_t rtp_flag_store(struct kobject *kobj, 
                             struct kobj_attribute *attr, 
                             const char *buf, size_t count)
{
        int ret; 
        u64 flag;
        ret = (u32)strict_strtoull(buf, 10, &flag);

        rtp_trace_flag = (u32)flag; 

        return count; 
}


static struct kobj_attribute rtp_port_attr =  __ATTR(rtp_port, 0644, rtp_port_show, rtp_port_store);
static struct kobj_attribute rtp_flag_attr =  __ATTR(rtp_flag, 0644, rtp_flag_show, rtp_flag_store);

static struct attribute *rtp_trace_attrs[] = {
        &rtp_port_attr.attr,
        &rtp_flag_attr.attr,
        NULL
};

static struct attribute_group rtp_trace_attr_group = {
	.attrs = rtp_trace_attrs,
};

#endif


static __init int sysfs_ipv4_init(void)
{
	struct kobject *ipv4_kobject;
	int ret;

	ipv4_kobject = kobject_create_and_add("ipv4", kernel_kobj);
	if (!ipv4_kobject)
		return -ENOMEM;

	ret = sysfs_create_group(ipv4_kobject, &ipv4_attr_group);
	if (ret) {
		kobject_put(ipv4_kobject);
		return ret;
	}

        #ifdef CONFIG_MP_DEBUG_TOOL_RTP_TRACE
        struct kobject *rtp_kobject;

	rtp_kobject = kobject_create_and_add("rtp_trace", kernel_kobj);
	if (!rtp_kobject)
		return -ENOMEM;

	ret = sysfs_create_group(rtp_kobject, &rtp_trace_attr_group);
	if (ret) {
		kobject_put(rtp_kobject);
		return ret;
	}
        #endif

	return 0;
}

subsys_initcall(sysfs_ipv4_init);
