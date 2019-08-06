# RISC-V: #/'Alphanumeric Shellcoding

```
By Hаdrien Ваrrаl, Rémi Géraud-Stewart, Georges-Axel Jaloyan, and David Naccache
```

This work has been presented at
 [WOOT'19](https://www.usenix.org/conference/woot19/) ([paper](https://www.usenix.org/conference/woot19/presentation/barral))
 and [DEFCON27](https://www.defcon.org/html/defcon-27/dc-27-index.html) ([slides](https://xn--fda.fr/riscv-alphanumeric-shellcoding/defcon27_riscv-alphanumeric-shellcoding.pdf))

## Overview

This tool helps design RISC-V (specifically RV64GC) shellcodes capable of running arbitrary code, whose ASCII binary representation
use only letters a–zA–Z, digits 0–9, and either of the three characters: `#`, `/`, `’`.

It consists of an alphanumeric (+1 character) unpacker. For any target shellcode (non-alphanumeric),
the tool will produce an alphanumeric (+1 character) shellcode with the unpacker and the packed version of your shellcode.
Run it on a RISC-V simulator/cpu and enjoy!

For a general introduction on RISC-V shellcoding, you may read the [blog post by Thomas Karpiniec](https://thomask.sdf.org/blog/2018/08/25/basic-shellcode-in-riscv-linux.html).

Folder contents:
- `baremetal`: Full source code and QEMU demo for each of the three variants
- `fedora`: Demos running on Fedora on QEMU
- `hifiveu`: Demos running on the HiFive-Unleashed board
- `payload`: Source code of the payloads used
- `lists`: How we generated the available instructions
- `scripts` / `tools`: Various helpers

## Quick-try

Building the shellcodes requires to build a RISC-V toolchain from source.
We only provide easy-to-test pre-built baremetal shellcodes.

The only prerequisite is having a RISC-V QEMU v4.0.0 or newer [https://www.qemu.org/](https://www.qemu.org/)

Then:
  - `cd riscv-alphanumeric-shellcoding/baremetal/prebuilt`
  - `cat hash.bin` (optional, to print the shellcode)
  - `sh launch_hash`   use Ctrl+A then X to exit
  - `cat slash.bin` (optional, to print the shellcode)
  - `sh launch_slash`   use Ctrl+A then X to exit
  - `cat tick.bin` (optional, to print the shellcode)
  - `sh launch_tick`   use Ctrl+A then X to exit
 
## Building && Testing

Start by cloning the repository:
```
git clone https://github.com/RischardV/riscv-alphanumeric-shellcoding.git
cd riscv-alphanumeric-shellcoding
```

#### Prerequisites:

__/!\ Warning: unless you are using a rolling-release distribution, you probably will not be able to simply install packages.
   You will need to install yourself the tools below:__
- RISC-V toolchain: install [https://github.com/riscv/riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain)
- RISC-V QEMU: install QEMU v4.0.0 or newer [https://www.qemu.org/](https://www.qemu.org/)

### QEMU bare-metal shellcodes

- Build the instructions lists (takes some time)
  - `cd riscv-alphanumeric-shellcoding/baremetal/lists`
  - `make`
- Build the shellcodes
  - `cd riscv-alphanumeric-shellcoding/baremetal/hash`
  - `make`
  - `cd riscv-alphanumeric-shellcoding/baremetal/slash`
  - `make`
  - `cd riscv-alphanumeric-shellcoding/baremetal/tick`
  - `make`
- Run the shellcodes
  - `cd riscv-alphanumeric-shellcoding/baremetal/hash`
  - `cat shellcode.bin` (optional, to print the shellcode)
  - `sh l`   use Ctrl+A then X to exit
  - `cd riscv-alphanumeric-shellcoding/baremetal/slash`
  - `cat shellcode.bin` (optional, to print the shellcode)
  - `sh l`   use Ctrl+A then X to exit
  - `cd riscv-alphanumeric-shellcoding/baremetal/tick`
  - `cat shellcode.bin` (optional, to print the shellcode)
  - `sh l`   use Ctrl+A then X to exit
Expected results:
  The string "Hello, world!" should print on the screen.

### QEMU Linux shellcodes

Prerequisites: A Fedora 28 Linux image running in a QEMU riscv environment (see [here](https://fedorapeople.org/groups/risc-v/disk-images/) and
  [here](https://wiki.qemu.org/Documentation/Platforms/RISCV#Booting_64-bit_Fedora)). 

- Build the shellcodes
  - `cd riscv-alphanumeric-shellcoding/fedora`
  - `make`
- Run the shellcodes
  - Start your Fedora RISC-V virtual machine
  - From the *host*: Send the compiled files `riscv-alphanumeric-shellcoding/fedora/{out,build/vuln.bin}` to the virtual machine (e.g. using scp)
  - On the *guest* Fedora VM: run the shellcodes using:
    * Hello word shellcodes:
        - `./vuln.bin < out/hello_hash.txt` for the 'hash' flavored shellcode
        - `./vuln.bin < out/hello_slash.txt` for the 'slash' flavored shellcode
        - `./vuln.bin < out/hello_tick.txt` for the 'tick' flavored shellcode
    
	Expected results:
          the string "Hello, world from shellcode!\n" should print on stdout

    * Execve /bin/sh shellcodes:
        - `(cat out/shell_hash.txt; echo ""; cat) | ./vuln.bin` for the 'hash' flavored shellcode
        - `(cat out/shell_slash.txt; echo ""; cat) | ./vuln.bin` for the 'slash' flavored shellcode
        - `(cat out/shell_tick.txt; echo ""; cat) | ./vuln.bin` for the 'tick' flavored shellcode
	
	Expected results:
          a shell should spawn with no prompt. To test it, type any command (e.g. `id`) and press enter. To exit the shell, type exit and then press enter.

    * Printing /etc/shadow shellcodes:
        - `./vuln.bin < out/shadow_hash.txt` for the 'hash' flavored shellcode
        - `./vuln.bin < out/shadow_slash.txt` for the 'slash' flavored shellcode
        - `./vuln.bin < out/shadow_tick.txt` for the 'tick' flavored shellcode
	
	Expected results:
	  the contents of the shadow file should be printed on stdout

    * Custom payloads (section 5.3 of the paper). You may modify the payload located in the `riscv_alphanumeric/payload` directory.

### HiFive Unleashed Linux shellcodes

Prerequisites: 
 - A HiFive-Unleashed board: [https://www.sifive.com/boards/hifive-unleashed](https://www.sifive.com/boards/hifive-unleashed)
 - The HiFive toolchain: [https://github.com/sifive/freedom-u-sdk](https://github.com/sifive/freedom-u-sdk)

- Build the shellcodes
  - `cd riscv-alphanumeric-shellcoding/hifiveu`
  - `make`

Running instructions are very similar to QEMU Linux shellcodes above. Refer to them.

## Documentation

Our academic paper gives a lot of details about design choices.
We encourage you to read it if you want to understand how the code works.

[Link to paper](https://xn--fda.fr/riscv-alphanumeric-shellcoding/preprint_riscv-alphanumeric-shellcoding.pdf)

## License

This tool is released under MIT license. See `LICENSE` file.
