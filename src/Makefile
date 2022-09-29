OBJS=main.o graphics.o console.o font.o hankaku.o

CPPFLAGS += -I.
CFLAGS  += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
					 -fno-exceptions -fno-rtti -std=c++17
LDFLAGS += --entry main -z norelro --image-base 0x100000 --static

.PHONY: all

all: kernel.elf
	mkdir -p ../image
	cp kernel.elf ../image

kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

%.o: %.cpp Makefile
	clang++ $(CFLAGS) $(CPPFLAGS) -c $< -o $@
