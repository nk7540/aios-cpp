run:
				cp loader/main.efi image/EFI/BOOT/BOOTX64.EFI
				qemu-system-x86_64 -bios ovmf/OVMF.fd -hda fat:rw:image

commit:
				git submodule update
				git add .
				git diff HEAD --color=always | less -R
				git commit
