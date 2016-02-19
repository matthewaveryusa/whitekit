Whitekit
========

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
