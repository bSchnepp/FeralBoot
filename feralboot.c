/*
	One big C file.
	No bloat,
	Just load an ELF executable called 'floskrnl.pro', let it have free reign over the system, etc.
	We'll deal with more stuff about it later and all that fun stuff.

	We're a boot loader, not a boot manager + boot loader. We'd like to support chainloading and having a menu (and thus being a boot manager too), but not for now.

	Just because we *really* like a black and yellow color combination for whatever reason (it just feels cool and is unique???), when we do have a menu, ensure highlights are yellow, text in a 
	decent kinda mid grey (selected as white), and everything some tone of black or something.
	Maybe use red somewhere for variation too? I'm not a UX designer so I don't know how well that would go out. Can just change later.
	We'd like to ____really____ avoid white wherever possible. White should be reserved for text. Maybe for important stuff..? We want black + yellow (and a litle red) on everything.

	Maybe we can pick up on that convergence dream of having one application run on a million different architectures and look 'native' on all of them?
	(Particularly with phone & PC, a game that runs on your PC should also be able to run on your phone, be it streaming, or massive performance hit with an emulator/JIT recompiler)
	Loootttss of work (especially since I'm the only one working on this), but hey, shoot a rocket at Andromeda and it will *eventually* get there.
 */

#include <efi.h>
#include <efilib.h>
#define E_IDENTC 16


//Global variables
EFI_SYSTEM_TABLE* SysTable;
EFI_BOOT_SERVICES* BootServices;
EFI_RUNTIME_SERVICES* RuntimeServices;



enum FeralBootStatus
{
	FERALBOOT_OK,
	FERALBOOT_UNKNOWN_ERROR,
	FERALBOOT_OUT_OF_MEMORY,
	FERALBOOT_WRONG_PARAMS,
	FERALBOOT_INVALID_HEADER,
	FERALBOOT_INVALID_ARCHITECTURE
};

typedef UINTN uintn_t;

void* malloc(uintn_t size)
{
	void* ptr;

	EFI_STATUS status = uefi_call_wrapper(BootServices->AllocatePool, 3, EfiLoaderData, size, &ptr);
	if (status != EFI_SUCCESS)
	{
		ptr = ((void*)0);
	}

	return ptr;
}

EFI_STATUS efialloc(uintn_t size, VOID** ptr)
{
	return uefi_call_wrapper(BootServices->AllocatePool, 3, EfiLoaderData, size, ptr);
}


EFI_STATUS efimemalloc(void)
{
	//TODO...
	//Strictly contiguous memory (in hardware), need so we can just point at kernel_main, shutdown BootServices, and jump.
}


//free() still works on EFI malloced stuff.
void free(void* pointer)
{
	uefi_call_wrapper(BootServices->FreePool, 1, pointer);	//EFI does it for us.
}



struct ElfHeader
{
	uint8_t e_ident[E_IDENTC];

	uint16_t e_type;
	uint16_t e_machine;

	uint32_t e_version;

	uintn_t e_entry;
	uintn_t e_phoff;
	uintn_t e_shoff;

	uint32_t e_flags;

	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
}ElfHeader;

struct ElfProgramHeader
{
	uint32_t p_type;
	uint32_t p_flags;
	
	uintn_t  p_offset;
	uintn_t  p_vaddr;
	uintn_t  p_paddr;
	uintn_t  p_filesz;
	uintn_t  p_memsz;
	uintn_t  p_align;
}ElfProgramHeader;

struct ElfSectionHeader
{
	uint32_t sh_name;
	uint32_t sh_type;
	
	uintn_t  sh_flags;
	uintn_t  sh_addr;
	uintn_t  sh_offset;
	uintn_t  sh_size;

	uint32_t sh_link;
	uint32_t sh_info;

	uintn_t  sh_addralign;
	uintn_t  sh_entsize;
}ElfSectionHeader;

enum ElfType
{
	ET_NONE = 0,
	ET_REL = 1,
	ET_EXEC = 2,
	ET_SHARED = 3,
	ET_CORE = 4	//This is used for program memory dumps
};





#if 0
//TODO: make all of this not garbage!
struct Kernel
{
	EFI_HANDLE handle;
	EFI_FILE_IO_INTERFACE* filesystem;
	EFI_FILE_HANDLE root;
	EFI_FILE_HANDLE kernel_file;	//Kernel we're pointing to.
}Kernel;

struct ElfInfo 
{
	struct ElfHeader* hdr;
	struct ElfProgramHeader* phdr;
	//TODO
}ElfInfo;


//TODO
struct Kernel_Logo
{
	int width;
	int height;
	int** colors;
	//TODO
}Kernel_Logo;

struct Configuration
{
	char* cmd_line_args;
	struct Kernel* kernel;
	struct Kernel_Logo* logo;
	int position;
}Configuration;
//TODO


//This stuff isn't necessary. Cut it out.

#endif


/**
 *	Checks the bit count of the natural registers of the processor. (32 vs 64-bit)
 *	ie, ARM32 returns 1 and x86-64 returns 2, AArch64 returns 2, etc. etc.
 *	Use this for that one field in e_ident that needs it.
 *
 *	@return 1 if 32-bit, 2 if 64-bit.
 */
uint8_t check_bit_size(void)
{
	if (sizeof(uintn_t) > 4)
	{
		return 2;
	}
	return 1;
}

//TODO: Refactor as a #define
uint8_t get_wanted_arch(void)
{
#if defined(__x86_64)
	return 62;	//ELF ID for x86-64
#elif defined(__i386__)
	return 3;	//ELF ID for x86
#endif
}

//TODO: AArch64/ARM32



enum FeralBootStatus CheckElfFile(struct ElfHeader* hdr)
{
	enum FeralBootStatus status = FERALBOOT_OK;
	uint8_t h0, h1, h2, h3;
	h0 = hdr->e_ident[0];
	h1 = hdr->e_ident[1];
	h2 = hdr->e_ident[2];
	h3 = hdr->e_ident[3];

	if ((h0 != 0x7F) || (h1 != 'E') || (h2 != 'L') || (h3 != 'F'))
	{
		return FERALBOOT_INVALID_HEADER;
		//This isn't an ELF executable!
	}

	uint8_t procarc = hdr->e_ident[7];
	if (procarc != get_wanted_arch())
	{
		return FERALBOOT_INVALID_ARCHITECTURE;
		//Wrong architecture!
		//Wishlist: emulator as raw EFI application and continue to run ELF kernel __ANYWAY__ on a virtual machine.
		//We'd do this through a JIT compiler, just like Redmond's WoW (ARM32 imitating 32-bit x86) does.
	}
	
	//TODO: rest of checks...
	return status;
}

EFI_STATUS Stall(int microseconds)
{
	return uefi_call_wrapper(BootServices->Stall, 1, microseconds);
}

EFI_STATUS Wait(int seconds)
{
	return Stall(seconds * 1000000);
}

EFI_STATUS ClrScr(void)
{
	return uefi_call_wrapper(SysTable->ConOut->ClearScreen, 1, SysTable->ConOut);
}


















CHAR8 cpu_vendor[12];
CHAR8 cpu_brand_name[48];
//ARCHITECTURE SPECIFIC STUFF!!!!!

#if defined(__x86_64) || defined(__x86) 
UINT32 registers_cache[4];
char items[4];

static void cpuid(int eax_val)
{
	__asm volatile("cpuid;"
			:"=a"(*(registers_cache+0)), "=b"(*(registers_cache+1)), "=c"(*(registers_cache+2)), "=d"(*(registers_cache+3))
			:"a"(eax_val)
	);
}

static char* read_register_cache(int position)
{
	
	for (int i = 0; i < 4; i++)
	{
		(items)[i] = (registers_cache[position] >> (i * 8)) & 0xFF;
	} 
	return (items);
}

static volatile void SetupCpuVendor(void)
{
	cpuid(0);
	char* two;
	char* three;
	char* four;

	two = read_register_cache(1);
	for (int i = 0; i < 4; i++)
	{
		cpu_vendor[i] = two[i];
	}

	three = read_register_cache(3);
	for (int i = 0; i < 4; i++)
	{
		cpu_vendor[i+4] = three[i];
	}

	four = read_register_cache(2);
	for (int i = 0; i < 4; i++)
	{
		cpu_vendor[i+8] = four[i];
	}
}

static volatile void SetupCpuBrandName(void)
{
	for (int iter = 0; iter < 3; iter++)
	{
		int eax = 0x80000002 + iter;
		cpuid(eax);
		
		for (int reg = 0; reg < 4; reg++)
		{
			char* cache = read_register_cache(reg);
			for (int pos = 0; pos < 4; pos++)
			{
				int offset_iter = 16 * iter;
				int offset_regr = reg * 4;
				cpu_brand_name[offset_iter + offset_regr + pos] = cache[pos]; 
			}
		}
	}
}

/*
static volatile void ReadCpuid(UINT32 info, UINT32* r1, UINT32* r2, UINT32* r3, UINT32* r4)
{
	*r1 = info;
	__asm volatile(
	".intel_syntax;\n"
	"mov %%eax, %[ri_];\n"
	"cpuid;\n"
	"mov %[r0_], %%eax;\n"
	"mov %[r1_], %%ebx;\n"
	"mov %[r2_], %%ecx;\n"
	"mov %[r3_], %%edx;\n"
	".att_syntax;"
	:[ri_]"=r"(*r1), [ri1_]"=r"(*r2), [ri2_]"=r"(*r3), [ri3_]"=r"(*r4)
	:[r0_]"r"(*r1),  [r1_]"r"(*r2),   [r2_]"r"(*r4),  [r3_]"r"(*r4)
	: 
	);
}*/
#endif


//TODO: CPUID for ARM... Performance doesn't matter. Implement with brute force assembly if necessary to act the same as x86.
//(If this feature doesn't exist, simply state "Unknown AArch64 chip" or "Unknown ARM32 chip"
//We don't really do "ARM desktop" or anything like that. ARM support is intended mainly for smartphones.
#if defined(__aarch64__)

static volatile void SetupCpuVendor(void)
{
	//TODO: AArch64 port
	return;
}

static volatile void SetupCpuBrandName(void)
{
	//TODO: AArch64 port, need to learn ARM32/AArch64 assembly.
	return;
}
#endif

#if defined(__arm__)

static volatile void SetupCpuVendor(void)
{
	//TODO: ARM32 port
	return;
}

static volatile void SetupCpuBrandName(void)
{
	//TODO: ARM32 port, need to learn ARM32/AArch64 assembly.
	return;
}

#endif

























EFI_STATUS efi_init_filesystems(void)
{
	EFI_STATUS status;
	EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	
	uintn_t handle_count = 0;
	EFI_HANDLE* handle_list;
	
	status = uefi_call_wrapper(BootServices->LocateHandleBuffer, 5, ByProtocol, &guid, NULL, &handle_count, &handle_list);
	//TODO...	
	return status;
}

int got_architecture;	//Architecture LoadKernel finds in ELF header.

void* LoadKernel(EFI_HANDLE file)
{
	//We'll need to mmap things to get this just right.
	

	//First, we load the file...
	
	struct ElfHeader* header;
	enum ElfType type;
	struct ElfProgramHeader** prog_header;
	struct ElfSectionHeader** sect_header;

	//TODO...
}









EFI_STATUS EFIAPI efi_main(EFI_HANDLE img_hdl, EFI_SYSTEM_TABLE* system_table) 
{
	InitializeLib(img_hdl, system_table);
	SysTable = system_table;
	BootServices = SysTable->BootServices;
	RuntimeServices = SysTable->RuntimeServices;
	
	ClrScr();
	Print(L"Initializing FeralBoot...\n");
	Print(L"Wanted architecture:    %u\n", get_wanted_arch());
	Print(L"UEFI Version:           %d.%02d\n", SysTable->Hdr.Revision >> 16, SysTable->Hdr.Revision & 0xFFFF);
	


	Print(L"Firmware Vendor:        %s, Version: %d.%02d\n", SysTable->FirmwareVendor, SysTable->FirmwareRevision >> 16, SysTable->FirmwareRevision & 0xFFFF);
	
	SetupCpuVendor();
	Print(L"CPU Vendor:             ", cpu_vendor);
	for (int i = 0; i < 12; i++)
	{
		Print(L"%c", cpu_vendor[i]); 
	}
	Print(L"\n");

	SetupCpuBrandName();
	Print(L"CPU Brand name:   ");
	for (int i = 0; i < 48; i++)
	{
		Print(L"%c", cpu_brand_name[i]); 
	}
	Print(L"\n");
	
	//TODO, check for 'floskrnl.pro' in the ESP. If not there, check subfolders and present options menu.

	//Put in filesystem initialization and look for floskrnl.pro directly in ESP.

	Print(L"Preparing boot...");
	//TODO: Switch to nice menu, clear screen, and load up the actual boot menu we want.

	Wait(300);
	
	return EFI_SUCCESS;
}
