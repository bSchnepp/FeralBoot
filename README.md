# FeralBoot

FeralBoot is the (U)EFI-compliant bootloader for Waypoint.
It is __separate__ from Waypoint so that it can be used for other operating systems or otherwise reused.

FeralBoot currently can only load kernels also located in the ESP. (until FS drivers can be made.)
___

FeralBoot's first few builds should export as 'BOOTX64.EFI' and be placed in an ESP. Later on, I'll write some scripts to have it moved out and be it's own entry in the system's default boot manager.
___

TODOS:

	- ABSOLUTELY EVERYTHING.
	
	- Keep GNU-EFI as a dependency, but anything that's not BSD licensed will NOT be used. Inject source & rebuild if necessary into the library. (Keep GPL away from WayBootMgr.)
	
	- Support ext2 at all. Possibly NTFS?
	
	- Support multiboot 1 and/or 2. Maybe?
	
	- Support running ELF executables as kernels, ala GNU GRUB.
	
	- Get it to boot a hypothetical Feral kernel
___

COMPLETED:
	
	- Identify CPU vendor and brand
	
	- Helper functions

	- Identify UEFI vendor and version

	- Essentially nothing

___

I'm trying to rush this, so we can get to kernel programming (the fun part!)
As such, expect code quality to be fairly OK to outright bad. As long as it works, loads the kernel, and runs it, that's all that matters for now.
We'll just refactor and clean up later.

Cureently compiles, and runs on test hardware (N3700 @ 1.60Ghz). 
Currently no actual booting functionality.
___

## License
FeralBoot (it's own sources) is distributed under the terms of the MIT license. See 'LICENSE' for details.
FeralBoot requires external software (gnu-efi) however, which is distributed under the terms of the BSD 3-clause license.
See 'LICENSE_THIRDPARTY' for details on that code. That file should be distributed with binary releases, for legal reasons.
(Need to go through and grep through all the files in gnu-efi, so probably more than that is needed.)

I'll end up replacing GNU-EFI with a clang-oriented alternative sometime.
