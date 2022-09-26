all: main.efi

%.efi: %.o
				lld-link /subsystem:efi_application /entry:EfiMain /out:$@ $<

%.o: %.c
				clang -target x86_64-pc-win32-coff -c $<
