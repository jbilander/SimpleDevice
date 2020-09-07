# Amiga SimpleDevice
Amiga Simple Device skel in C using Bebbo's or Bartman's gcc.
***
This example shows you how to build a simple device driver for the Amiga in C with a modern cross compile gcc-toolchain for the Amiga using VSCode as IDE. Driver should work with 1.3 and above. I haven't tested with older versions.
***
The `Makefile` is made to work with both `Bebbo's amiga-gcc toolchain` as well as `Bartman^Abyss' amiga-elf gcc toolchain`. Currently it is set by default to Bebbo's toolchain, but you can easily switch by commenting out `hunk-toolchain` and uncomment `elf-toolchain` in the Makefile. Obviously you need to install "your" prefered toolchain for it to work and set the correct path to compiler and some settings in .vscode for intellisense and compilation to work. The Makefile should also work with Bartman's `gnumake.exe` running natively on Windows as well as `make` working on MSYS2 and on Linux. The `device.c` is only made to compile with Bebbo's toolchain since it is using register assignments to function parameters, no such thing exists in the elf-toolchain but you can work around it using inline asm should you want to try out building with the elf-toolchain instead. The `Makefile` is also set to automatically disassemble the build so that the assembly generated can easily be inspected. If you are using the elf-toolchain and have Bebbo's toolchain installed as well you can uncomment the `m68k-amigaos-objdump -D $(DIR)/$(FILENAME) > $(DIR)/$(FILENAME).s` line and you will have an disassembly of the executable hunk-file generated after the `elf2hunk` step as well.
***
The Makfile is set to delete and recompile the executable if any source files `(%.c or %.s)` have been touched before running `make`. Just comment out the `@$(DEL_EXE)` on both `%.c and %.s` if you don't want it to work this way.
***
### Some screenshots:
On Windows with MSYS2:
<br />
<a href="images/VSCode_on_windows_with_MSYS2.jpg">
<img src="images/VSCode_on_windows_with_MSYS2.jpg" width="577" height="366">
</a>
<br />

On Linux Mint:
<br />
<a href="images/VSCode_on_linux.jpg">
<img src="images/VSCode_on_linux.jpg" width="577" height="369">
</a>

***

### Make commands:
* build debug: `make debug`  (This created a folder `build-debug` with all the built files in it, compiles with debug-flags set)
* build release: `make`  (This created a folder `build-release` with all the built files in it, compiles with release-flags set)
* clean debug: `make cleandebug`
* clean release: `make cleanrelease`
* clean both: `make clean`

***

### "Debug" with KPrintF in methods running in Forbidden mode by Exec:
As you might know `Init_Device/Open/Close/Expunge` runs single threaded by Exec, no other task is allowed to run in this mode so no writing to a dos-console possible as it would mean deadlock to occur, however with the `debug.lib (libdebug.a)` linked into our build (with `-ldebug -mcrt=clib2`) it is possible to get `KPrintF`-statements out on the serial port terminal. Good thing is WinUAE has an option to emulate the serial terminal of the Amiga and then we can use telnet to connect to it with the command `telnet localhost 1234`

Enable WinUAE serial port on 1234, and open a cmd-prompt and write the telnet command (obviously you'll need telnet installed first via `Control Panel->Program and Features->Turn windows features on/off)`.
<br />
<br />
<a href="images/telnet_to_WinUAE_emulated_serial_port.jpg">
<img src="images/telnet_to_WinUAE_emulated_serial_port.jpg" width="577" height="319">
</a>
***
For conveniency, do also map the Project folder in WinUAE so we easily can copy our newly built device driver over to the emulated Amiga:
<br /><br />
<a href="images/project_folder_mapped_as_drive_in_WinUAE.jpg">
<img src="images/project_folder_mapped_as_drive_in_WinUAE.jpg" width="577" height="432">
</a>
***
With dopus on the Amiga you can copy the simple.device into devs.
<br /><br />
<a href="images/project_folder_shown_in_dopus.jpg">
<img src="images/project_folder_shown_in_dopus.jpg" width="577" height="493">
</a>
<br /><br />
<a href="images/copy_driver_to_devs_with_dopus.jpg">
<img src="images/copy_driver_to_devs_with_dopus.jpg" width="577" height="493">
</a>
***
To load the device we need a device list with the Device entry pointing to our name of the file `simple.device`, I'll use a `SD0`-file here made in `Devs` as a dummy to see if it will work to load the driver, Exec will scan for a romtag in our device driver:
<br /><br />
<a href="images/mapping_device_name_in device_list.jpg">
<img src="images/mapping_device_name_in device_list.jpg" width="577" height="493">
</a>
***
And with telnet connected and running by double-clicking the `SD0`-file it is indeed showing `KPrintF`-output on the serial console...yay!
<br /><br />
<a href="images/telnet_kprintf_output.jpg">
<img src="images/telnet_kprintf_output.jpg" width="577" height="418">
</a>
***
In the disassembly we can also se that the romtag is created correctly after `moveq #-1,d0` `rts` as it should be:
<br /><br />
<a href="images/disassembly_screenshot.jpg">
<img src="images/disassembly_screenshot.jpg" width="577" height="364">
</a>
***
That's all folks!, Happy Amiga Hackin'
