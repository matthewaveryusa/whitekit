#include "data.h"

MODULE_LICENSE("GPL");

int module_handler ( struct notifier_block *nblock, unsigned long event, void *ptr )
{
  struct module *new_module = ptr;
  struct scatterlist sg;
  struct hash_desc desc;
  struct module_details d;
  unsigned long flags;
  int i;
  DEFINE_SPINLOCK(module_event_spinlock);

  spin_lock_irqsave(&module_event_spinlock, flags);
  if(new_module->state ==  MODULE_STATE_COMING) {
    desc.tfm = crypto_alloc_hash("sha256", 0, CRYPTO_ALG_ASYNC);
    crypto_hash_init(&desc);
    sg_init_one(&sg, new_module->module_core, new_module->core_size);
    crypto_hash_update(&desc, &sg, new_module->core_size);
    memcpy(d.name,new_module->name,256);
    d.name[255] = '\0';
    spin_unlock_irqrestore(&module_event_spinlock, flags);
    crypto_hash_final(&desc, d.hash);
    crypto_free_hash(desc.tfm);
    printk("loaded module '%s' sha256sum: ", d.name);
    for (i = 0; i < 256/8; i++)
      printk("%02X", (unsigned int)(d.hash[i] & 0xFF));
    printk("\n");
  }
  spin_unlock_irqrestore(&module_event_spinlock, flags);
  return NOTIFY_DONE;
}

struct notifier_block nb = {
  .notifier_call = module_handler,
  .priority = INT_MAX,
};

void monitor_module_loading( void )
{
  register_module_notifier(&nb);
}

void stop_monitor_module_loading ( void )
{
  unregister_module_notifier(&nb);
}

#if defined(_CONFIG_X86_)
// Phrack #58 0x07; sd, devik
unsigned long *find_sys_call_table ( void )
{
  char **p;
  unsigned long sct_off = 0;
  unsigned char code[255];

  asm("sidt %0":"=m" (idtr));
  memcpy(&idt, (void *)(idtr.base + 8 * 0x80), sizeof(idt));
  sct_off = (idt.off2 << 16) | idt.off1;
  memcpy(code, (void *)sct_off, sizeof(code));

  p = (char **)memmem(code, sizeof(code), "\xff\x14\x85", 3);

  if ( p )
    return *(unsigned long **)((char *)p + 3);
  else
    return NULL;
}
#elif defined(_CONFIG_X86_64_)
// http://bbs.chinaunix.net/thread-2143235-1-1.html
unsigned long *find_sys_call_table ( void )
{
  char **p;
  unsigned long sct_off = 0;
  unsigned char code[512];

  rdmsrl(MSR_LSTAR, sct_off);
  memcpy(code, (void *)sct_off, sizeof(code));

  p = (char **)memmem(code, sizeof(code), "\xff\x14\xc5", 3);

  if ( p )
  {
    unsigned long *sct = *(unsigned long **)((char *)p + 3);

    // Stupid compiler doesn't want to do bitwise math on pointers
    sct = (unsigned long *)(((unsigned long)sct & 0xffffffff) | 0xffffffff00000000);

    return sct;
  }
  else
    return NULL;
}

// Obtain sys_call_table on amd64; pouik
unsigned long *find_ia32_sys_call_table ( void )
{
  char **p;
  unsigned long sct_off = 0;
  unsigned char code[512];

  asm("sidt %0":"=m" (idtr));
  memcpy(&idt, (void *)(idtr.base + 16 * 0x80), sizeof(idt));
  sct_off = (idt.off2 << 16) | idt.off1;
  memcpy(code, (void *)sct_off, sizeof(code));

  p = (char **)memmem(code, sizeof(code), "\xff\x14\xc5", 3);

  if ( p )
  {
    unsigned long *sct = *(unsigned long **)((char *)p + 3);

    // Stupid compiler doesn't want to do bitwise math on pointers
    sct = (unsigned long *)(((unsigned long)sct & 0xffffffff) | 0xffffffff00000000);

    return sct;
  }
  else
    return NULL;
}
#endif

void *get_inet_ioctl ( int family, int type, int protocol )
{
  void *ret;
  struct socket *sock = NULL;

  if ( sock_create(family, type, protocol, &sock) )
    return NULL;

  ret = sock->ops->ioctl;

  sock_release(sock);

  return ret;
}

void *get_vfs_iterate ( const char *path )
{
  void *ret;
  struct file *filep;

  if ( (filep = filp_open(path, O_RDONLY, 0)) == NULL )
    return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
  ret = filep->f_op->readdir;
#else
  ret = filep->f_op->iterate;
#endif


  filp_close(filep, 0);

  return ret;
}

void *get_tcp_seq_show ( const char *path )
{
  void *ret;
  struct file *filep;
  struct tcp_seq_afinfo *afinfo;

  if ( (filep = filp_open(path, O_RDONLY, 0)) == NULL )
    return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
  afinfo = PDE(filep->f_dentry->d_inode)->data;
#else
  afinfo = PDE_DATA(filep->f_path.dentry->d_inode);
#endif
  ret = afinfo->seq_ops.show;

  filp_close(filep, 0);

  return ret;
}

void *get_udp_seq_show ( const char *path )
{
  void *ret;
  struct file *filep;
  struct udp_seq_afinfo *afinfo;

  if ( (filep = filp_open(path, O_RDONLY, 0)) == NULL )
    return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
  afinfo = PDE(filep->f_dentry->d_inode)->data;
#else
  afinfo = PDE_DATA(filep->f_path.dentry->d_inode);
#endif
  ret = afinfo->seq_ops.show;

  filp_close(filep, 0);

  return ret;
}

void state_populate(struct state *s) {
  s->proc_iterate       = get_vfs_iterate("/proc");
  s->root_iterate       = get_vfs_iterate("/");
  s->tcp4_seq_show      = get_tcp_seq_show("/proc/net/tcp");
  s->tcp6_seq_show      = get_tcp_seq_show("/proc/net/tcp6");
  s->udp4_seq_show      = get_udp_seq_show("/proc/net/udp");
  s->udp6_seq_show      = get_udp_seq_show("/proc/net/udp6");
  s->inet_ioctl         = get_inet_ioctl(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  s->dev_get_flags      = &dev_get_flags;

#if defined(_CONFIG_X86_64_)
  s->ia32_sys_call_table = find_ia32_sys_call_table();
#endif
  s->sys_call_table = find_sys_call_table();

  if(s->proc_iterate  != NULL )  memcpy(s->proc_iterate_instruct ,  s->proc_iterate, INSPECT_SIZE);
  if(s->root_iterate  != NULL )  memcpy(s->root_iterate_instruct ,  s->root_iterate, INSPECT_SIZE);
  if(s->tcp4_seq_show != NULL )  memcpy(s->tcp4_seq_show_instruct, s->tcp4_seq_show, INSPECT_SIZE);
  if(s->tcp6_seq_show != NULL )  memcpy(s->tcp6_seq_show_instruct, s->tcp6_seq_show, INSPECT_SIZE);
  if(s->udp4_seq_show != NULL )  memcpy(s->udp4_seq_show_instruct, s->udp4_seq_show, INSPECT_SIZE);
  if(s->udp6_seq_show != NULL )  memcpy(s->udp6_seq_show_instruct, s->udp6_seq_show, INSPECT_SIZE);
  if(s->inet_ioctl    != NULL )  memcpy(s->dev_get_flags_instruct, s->dev_get_flags, INSPECT_SIZE);
  if(s->dev_get_flags != NULL )  memcpy(s->inet_ioctl_instruct   , s->inet_ioctl,    INSPECT_SIZE);
}

void print_buf(const char* label, const char* buffer, int size) {
  int i;
  printk("%s",label);
  if(buffer == NULL) {
    printk("??");
  } else {
    for (i = 0; i < size; i++)
      printk("\\X%02X", (unsigned int)(buffer[i] & 0xFF));
  }
  printk("\n");
}

void print_buf_diff(const char* label, const char* buffer, const char* buffer2, int size) {
  int i;
  if(buffer == NULL || buffer2 == NULL) {
    printk("%s",label);
    printk("??");
  } else {
    for (i = 0; i < size; i++) {
      if(buffer[i] != buffer2[i]) break;
    }
    if(i == size) return;
    printk("%s",label);
    for (i = 0; i < size; i++)
      printk("\\X%02X ", (unsigned int)(buffer[i] & 0xFF));
    printk(" -> ");
    for (i = 0; i < size; i++)
      printk("\\X%02X ", (unsigned int)(buffer2[i] & 0xFF));
  }
  printk("\n");
}


void state_print(const struct state *s) {
  printk("sys_call_table:     0x%p\n",s->sys_call_table);
  printk("ia32_sys_call_table:0x%p\n",s->ia32_sys_call_table);
  printk("proc_iterate:       0x%p\n",s->proc_iterate);
  printk("root_iterate:       0x%p\n",s->root_iterate);
  printk("tcp4_seq_show:      0x%p\n",s->tcp4_seq_show);
  printk("tcp6_seq_show:      0x%p\n",s->tcp6_seq_show);
  printk("udp4_seq_show:      0x%p\n",s->udp4_seq_show);
  printk("udp6_seq_show:      0x%p\n",s->udp6_seq_show);
  printk("dev_get_flags:      0x%p\n",s->dev_get_flags);
  printk("inet_ioctl:         0x%p\n",s->inet_ioctl);

  print_buf("proc_iterate:  ",s->proc_iterate_instruct ,INSPECT_SIZE);
  print_buf("root_iterate:  ",s->root_iterate_instruct ,INSPECT_SIZE);
  print_buf("tcp4_seq_show: ",s->tcp4_seq_show_instruct,INSPECT_SIZE);
  print_buf("tcp6_seq_show: ",s->tcp6_seq_show_instruct,INSPECT_SIZE);
  print_buf("udp4_seq_show: ",s->udp4_seq_show_instruct,INSPECT_SIZE);
  print_buf("udp6_seq_show: ",s->udp6_seq_show_instruct,INSPECT_SIZE);
  print_buf("dev_get_flags: ",s->dev_get_flags_instruct,INSPECT_SIZE);
  print_buf("inet_ioctl:    ",s->inet_ioctl_instruct   ,INSPECT_SIZE);

}

void state_diff(const struct state *s, const struct state *n) {
  if(s->sys_call_table != n->sys_call_table) printk("sys_call_table:     0x%p -> 0x%p\n",s->sys_call_table, n->sys_call_table);
  if(s->ia32_sys_call_table != n->ia32_sys_call_table) printk("ia32_sys_call_table:0x%p -> 0x%p\n",s->ia32_sys_call_table, n->ia32_sys_call_table);
  if(s->proc_iterate != n->proc_iterate) printk("proc_iterate:       0x%p -> 0x%p\n",s->proc_iterate, n->proc_iterate);
  if(s->root_iterate != n->root_iterate) printk("root_iterate:       0x%p -> 0x%p\n",s->root_iterate, n->root_iterate);
  if(s->tcp4_seq_show != n->tcp4_seq_show) printk("tcp4_seq_show:      0x%p -> 0x%p\n",s->tcp4_seq_show, n->tcp4_seq_show);
  if(s->tcp6_seq_show != n->tcp6_seq_show) printk("tcp6_seq_show:      0x%p -> 0x%p\n",s->tcp6_seq_show, n->tcp6_seq_show);
  if(s->udp4_seq_show != n->udp4_seq_show) printk("udp4_seq_show:      0x%p -> 0x%p\n",s->udp4_seq_show, n->udp4_seq_show);
  if(s->udp6_seq_show != n->udp6_seq_show) printk("udp6_seq_show:      0x%p -> 0x%p\n",s->udp6_seq_show, n->udp6_seq_show);
  if(s->dev_get_flags != n->dev_get_flags) printk("dev_get_flags:      0x%p -> 0x%p\n",s->dev_get_flags, n->dev_get_flags);
  if(s->inet_ioctl != n->inet_ioctl) printk("inet_ioctl:         0x%p -> 0x%p\n",s->inet_ioctl, n->inet_ioctl);

  print_buf_diff("proc_iterate:  ",s->proc_iterate_instruct ,n->proc_iterate_instruct ,INSPECT_SIZE);
  print_buf_diff("root_iterate:  ",s->root_iterate_instruct ,n->root_iterate_instruct ,INSPECT_SIZE);
  print_buf_diff("tcp4_seq_show: ",s->tcp4_seq_show_instruct,n->tcp4_seq_show_instruct,INSPECT_SIZE);
  print_buf_diff("tcp6_seq_show: ",s->tcp6_seq_show_instruct,n->tcp6_seq_show_instruct,INSPECT_SIZE);
  print_buf_diff("udp4_seq_show: ",s->udp4_seq_show_instruct,n->udp4_seq_show_instruct,INSPECT_SIZE);
  print_buf_diff("udp6_seq_show: ",s->udp6_seq_show_instruct,n->udp6_seq_show_instruct,INSPECT_SIZE);
  print_buf_diff("dev_get_flags: ",s->dev_get_flags_instruct,n->dev_get_flags_instruct,INSPECT_SIZE);
  print_buf_diff("inet_ioctl:    ",s->inet_ioctl_instruct   ,n->inet_ioctl_instruct   ,INSPECT_SIZE);

}


struct task_struct *task;
int thread_function(void *data)
{
  struct state *tmp;
  while(!kthread_should_stop()){
    state_populate(now);
    state_diff(prev,now);

    //swap
    tmp = prev;
    prev = now;
    now = tmp;

    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(10*HZ);
  }
  return 0;
}

static int __init begin_whitekit ( void ) {
  //list_del_init(&__this_module.list);
  //kobject_del(__this_module.holders_dir->parent);
  monitor_module_loading();
  state_populate(prev);
  state_print(prev);
  task = kthread_run(&thread_function,NULL,"whitekit-thread");
  return 0;
}

static void __exit end_whitekit ( void )
{
  kthread_stop(task);
  stop_monitor_module_loading();
}

module_init(begin_whitekit);
module_exit(end_whitekit);
