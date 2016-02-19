#pragma once
#include <linux/capability.h>
#include <linux/cred.h>
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/in.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/unistd.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <net/udp.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif



#define INSPECT_SIZE 12

struct module_details {
  char name[256];
  u8 hash[256/8];
};

struct state {
  void* sys_call_table;
  void* ia32_sys_call_table;
  void* proc_iterate;
  void* root_iterate;
  void* tcp4_seq_show;
  void* tcp6_seq_show;
  void* udp4_seq_show;
  void* udp6_seq_show;
  void* dev_get_flags;
  void* inet_ioctl;
  char proc_iterate_instruct[INSPECT_SIZE];
  char root_iterate_instruct[INSPECT_SIZE];
  char tcp4_seq_show_instruct[INSPECT_SIZE];
  char tcp6_seq_show_instruct[INSPECT_SIZE];
  char udp4_seq_show_instruct[INSPECT_SIZE];
  char udp6_seq_show_instruct[INSPECT_SIZE];
  char dev_get_flags_instruct[INSPECT_SIZE];
  char inet_ioctl_instruct[INSPECT_SIZE];
};

struct state s1 = {0}, s2 = {0};
struct state* prev = &s1, *now = &s2;

extern unsigned long *sys_call_table;
#if defined(_CONFIG_X86_64_)
extern unsigned long *ia32_sys_call_table;
#endif
    
struct {
    unsigned short limit;
    unsigned long base;
} __attribute__ ((packed))idtr;

struct {
    unsigned short off1;
    unsigned short sel;
    unsigned char none, flags;
    unsigned short off2;
} __attribute__ ((packed))idt;


void *memmem ( const void *haystack, size_t haystack_size, const void *needle, size_t needle_size )
{
    char *p;

    for ( p = (char *)haystack; p <= ((char *)haystack - needle_size + haystack_size); p++ )
        if ( memcmp(p, needle, needle_size) == 0 )
            return (void *)p;

    return NULL;
}

