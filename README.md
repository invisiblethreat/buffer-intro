# A Stack Buffer Overflow

### Overview

* This is a simple stack buffer overflow demonstration.
* Protections have been explicitly disabled in the compiler.
* The majority of this work is a reimplementation of [https://vimeo.com/33106013](https://vimeo.com/33106013)

### Further Reading
* [http://en.wikipedia.org/wiki/Data_Execution_Prevention](DEP)
* [http://en.wikipedia.org/wiki/Address_space_layout_randomization](ASLR)
* [http://www.dirac.org/linux/gdb/](The best intro to GDB)


### Walkthrough
1 Satisify your build dependencies. We're compiling to x86, so we need to have 32-bit libraries
  1 Lots of people will be running x64, so you might need to hunt down packages that look like `lib32gcc1`
1 Build your `overflow` by running `./build`
1 Enter the debugger with `gdb -q overflow`
