# NEMU

NEMU(NJU Emulator) is a simple but complete full-system emulator designed for teaching purpose.
Currently it supports x86, mips32, riscv32 and riscv64.
To build programs run above NEMU, refer to the [AM project](https://github.com/NJU-ProjectN/abstract-machine).

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with reference design (e.g. QEMU)
  * snapshot
* CPU core with support of most common used instructions
  * x86
    * real mode is not supported
    * x87 floating point instructions are not supported
  * mips32
    * CP1 floating point instructions are not supported
  * riscv32
    * only RV32IM
  * riscv64
    * only RV64IM
* memory
* paging
  * TLB is optional (but necessary for mips32)
  * protection is not supported
* interrupt and exception
  * protection is not supported
* 5 devices
  * serial, timer, keyboard, VGA, audio
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O

# ELF format
* e_ehsize = ELF header size
* e_phoff  = Program header table file offset
* e_phentsize = Program header table entry size
* e_phnum = Program header table entry count
* e_shoff = Section header table file offset
* e_shentsize = Section header table entry size
* e_shnum = Section header table entry count
* e_shstrndx = Section header string table index ???

0               ELF header
e_phoff         program header table 
            n * e_phentsize

e_shoff         Section header table
              |  section_name
              |  section_type
  e_shentsize |  section_offset
              |        ...
        n * e_shentsize
      


## run
`make -j run`
