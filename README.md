Whitekit
========
write to dmesg a report on changes to system calls typically done by rootkits. Sample output:

    [15784.484645] sys_call_table:     0xffffffff81801460
    [15784.484652] ia32_sys_call_table:0xffffffff8180d6c0
    [15784.484655] proc_iterate:       0xffffffff81252aa0
    [15784.484658] root_iterate:       0xffffffff812694b0
    [15784.484660] tcp4_seq_show:      0xffffffff817069e0
    [15784.484663] tcp6_seq_show:      0xffffffff81778c60
    [15784.484666] udp4_seq_show:      0xffffffff81711ea0
    [15784.484668] udp6_seq_show:      0xffffffff8176a930
    [15784.484671] dev_get_flags:      0xffffffff816a40e0
    [15784.484674] inet_ioctl:         0xffffffff8171e8a0
    [15784.484676] proc_iterate:  \X48\XB8\X50\XC0\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484693] root_iterate:  \X48\XB8\X00\XC0\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484708] tcp4_seq_show: \X48\XB8\X90\XC3\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484722] tcp6_seq_show: \X48\XB8\XB0\XC2\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484737] udp4_seq_show: \X48\XB8\XD0\XC1\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484751] udp6_seq_show: \X48\XB8\XF0\XC0\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484766] dev_get_flags: \X48\XB8\XA0\XC0\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    [15784.484780] inet_ioctl:    \X48\XB8\X50\XCE\X9E\XC0\XFF\XFF\XFF\XFF\XFF\XE0
    //rootkit installed and changes detected:
    [15794.478365] proc_iterate:  \X48 \XB8 \X50 \XC0 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X89 \XE5 \X41 \X54 \X49
    [15794.478404] root_iterate:  \X48 \XB8 \X00 \XC0 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X89 \XE5 \X41 \X57 \X41
    [15794.478437] tcp4_seq_show: \X48 \XB8 \X90 \XC3 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X89 \XE5 \X41 \X57 \X41
    [15794.478470] tcp6_seq_show: \X48 \XB8 \XB0 \XC2 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X89 \XE5 \X41 \X57 \X41
    [15794.478502] udp4_seq_show: \X48 \XB8 \XD0 \XC1 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X89 \XE5 \X41 \X57 \X41
    [15794.478535] udp6_seq_show: \X48 \XB8 \XF0 \XC0 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X48 \X83 \XFE \X01 \X48 \X89
    [15794.478567] dev_get_flags: \X48 \XB8 \XA0 \XC0 \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X0F \XB7 \X87 \X10 \X02 \X00 \X00
    [15794.478599] inet_ioctl:    \X48 \XB8 \X50 \XCE \X9E \XC0 \XFF \XFF \XFF \XFF \XFF \XE0  -> \X66 \X66 \X66 \X66 \X90 \X55 \X8D \X86 \XFA \X76 \XFF \XFF
    [15839.100987] loaded module 'suterusu' sha256sum: 695B2C06AB7DE9846D3632CC11405F5E851AFF444725E23D4AF9D820E7602131
                                                          

Typical compilation steps:

    $ wget http://kernel.org/linux-x.x.x.tar.gz
    $ tar xvf linux-x.x.x.tar.gz
    $ cd linux-x.x.x
    $ make menuconfig
    $ make modules_prepare
    $ cd /path/to/suterusu
    $ make linux-x86 KDIR=/path/to/kernel


To compile against the currently running kernel (kernel headers installed):

    $ make linux-x86 KDIR=/lib/modules/$(uname -r)/build

deeply inspired by Suterusu
