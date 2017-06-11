## Change as needed.
ARCH = x86_64

CC = gcc
## CC = clang ## Because of the way gnu-efi works, Feralboot MUST be built on gcc-supported systems (ie, Arch or Debian), because I'm not porting gcc to Waypoint. If you port gcc yourself, it's OK though.
## Waypoint wants to use clang over gcc, as much as possible, and want to avoid using GNU or GPL-licensed stuff whenever we possibly can, just for licensing consistency purposes.
LD = ld.lld  ## LD = ld
## And because lld works w/ gnu-efi...

OBJS = feralboot.o
TARGET = BOOTX64.efi

## Change as needed. Default because I use that on Linux.
INSTALL_LOC = /run/media/brian-dev/EFI/EFI/BOOT
## INSTALL_LOC = A:/Users/Brian/Devices/FeralBoot/EFI/EFI/BOOT

## Change as needed.
GNU_EFI_LOC = $$HOME/gnu-efi

EFI_INC      = $(GNU_EFI_LOC)/inc
EFI_LIB      = $(GNU_EFI_LOC)/$(ARCH)/gnuefi

EFI_PROTOCOL = $(EFI_INC)/protocol
EFI_ARCH     = $(EFI_INC)/$(ARCH)



CFLAGS = -DEFI_FUNCTION_WRAPPER -I$(EFI_INC) -I$(EFI_ARCH) -I$(EFI_PROTOCOL) -fno-stack-protector -fPIC -fshort-wchar -mno-red-zone -Wall 
LDFLAGS = -nostdlib -znocombreloc -T $(GNU_EFI_LOC)/gnuefi/elf_$(ARCH)_efi.lds -shared -Bsymbolic -L $(EFI_LIB) -L $(GNU_EFI_LOC)/$(ARCH) -L $(GNU_EFI_LOC)/$(ARCH)/lib $(EFI_LIB)/crt0-efi-$(ARCH).o

all: $(TARGET)

BOOTX64.so: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc --target=efi-app-$(ARCH) $^ $@

clean:
	rm -f *.efi *.so *.o

install: all
	cp $(TARGET) $(INSTALL_LOC)/BOOTX64.efi
