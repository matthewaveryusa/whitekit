obj-m += whitekit.o
whitekit-objs := main.o

default:
	@echo "To build whitekit:"
	@echo "  make TARGET KDIR=/path/to/kernel"
	@echo
	@echo "To clean the build dir:"
	@echo "  make clean KDIR=/path/to/kernel"
	@echo
	@echo "Supported targets:"
	@echo "linux-x86    	Linux, x86"
	@echo "linux-x86_64 	Linux, x86_64"
	@echo

linux-x86:
ifndef KDIR
	@echo "Must provide KDIR!"
	@exit 1
endif
	$(MAKE) ARCH=x86 EXTRA_CFLAGS="-D_CONFIG_X86_" -C $(KDIR) M=$(PWD) modules

linux-x86_64:
ifndef KDIR
	@echo "Must provide KDIR!"
	@exit 1
endif
	$(MAKE) ARCH=x86_64 EXTRA_CFLAGS="-D_CONFIG_X86_64_" -C $(KDIR) M=$(PWD) modules

clean:
ifndef KDIR
	@echo "Must provide KDIR!"
	@exit 1
endif
	$(MAKE) -C $(KDIR) M=$(PWD) clean
