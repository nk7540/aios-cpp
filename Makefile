SHELL=/bin/bash

THIS_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

.PHONY: load,kernel,run,commit

all:
	make load
	make kernel
	make run

load: LoaderPkg/Main.c
	@if [ ! -d edk2 ]; then \
		git clone https://github.com/tianocore/edk2.git ; \
		git submodule update --init --recursive ; \
		make -C edk2/BaseTools ; \
	fi
	ln -sf $(THIS_DIR)LoaderPkg edk2
	cd edk2; source edksetup.sh;\
	build -p LoaderPkg/LoaderPkg.dsc -b DEBUG -a X64 -t CLANG38 -v
	mkdir -p image/EFI/BOOT
	cp edk2/Build/LoaderX64/DEBUG_CLANG38/X64/Loader.efi image/EFI/BOOT/BOOTX64.EFI

kernel:
	make -C src

run:
	qemu-system-x86_64 -bios ovmf/OVMF.fd -hda fat:rw:image -monitor stdio

debug:
	qemu-system-x86_64 -bios ovmf/OVMF.fd -hda fat:rw:image -monitor stdio -s -S

commit:
	git submodule update
	git add .
	git diff HEAD --color=always | less -R
	git commit

.FORCE:
