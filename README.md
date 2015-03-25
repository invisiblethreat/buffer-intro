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

1. Satisify your build dependencies. We're compiling to x86, so we need to have 32-bit libraries
  1. Lots of people will be running x64, so you might need to hunt down packages that look like `lib32gcc1`
1. Build your `overflow` by running `./build`
1. Optional - edit `~/.gdbinit` to allow the loading of the repo `.gdbinit` with `add-auto-load-safe-path <path to repo/.gdbinit`
1. Enter the debugger with `gdb -q overflow`
1. List the code with `list`. This is available due to our debugging information provided by the `-g` flag when compiling
```
(gdb) list
1       #include "string.h"
2       #include "stdio.h"
3
4       void cold_code()
5       {
6           printf("%s\n", "This is cold code");
7       }
8
9       void very_cold_code()
10      {
11          printf("%s\n", "This is very cold code");
12      }
13
14      void strcopy_is_the_devil(char *str)
15      {
16          char input[3];
17          strcpy(input, str);
18      }
19
20      void main(int argc, char *argv[])
21      {
22          strcopy_is_the_devil(argv[1]);
23          printf("Exiting cleanly.\n");
24      }
(gdb)
```
1. We now want to set a breakpoint at line 17, before the strcopy
```
(gdb) b 17
Breakpoint 1 at 0x8048461: file buffer-intro.c, line 17.
```
1. Let's run an automated command when we hit this break point(prompts removed so you can cut and paste)
```
command
printf "Before strcopy - 32 hex words starting at $esp\n"
x/32xw $esp
printf "\n"
printf "Address and contents of $ebp: "
x/1xw $ebp
printf "\n"
end
```
1. We now want to set a breakpoint at line 18, after the strcopy
```
(gdb) b 18
Breakpoint 4 at 0x8048473: file buffer-intro.c, line 18.
```
1. We want to run the same commands automatically after the strcopy
```
command
printf "After strcopy - 32 hex words starting at $esp\n"
x/32xw $esp
printf "\n"
printf "Address and contents of $ebp: "
x/1xw $ebp
printf "\n"
end
```
1. Now that we have breakpoints set, let's run the binary with some input
```
(gdb) run AAA
Starting program: /home/walsh/git/buffer-intro/overflow AAA
```
1. We've hit the breakpoint and the associated commands have been executed
```
Breakpoint 3, strcopy_is_the_devil (str=0xffffdc4c "AAA") at buffer-intro.c:17
/home/walsh/git/buffer-intro/buffer-intro.c:17:234:beg:0x8048461
Before strcopy - 32 hex words starting at $esp
0xffffd9e0:     0x00000000      0x09ca212c      0x00000001      0x080482bd
0xffffd9f0:     0xffffdc26      0x0000002f      0xffffda18      0x08048499
0xffffda00:     0xffffdc4c      0xffffdac4      0xffffdad0      0xf7e4b39d
0xffffda10:     0xf7fc03c4      0xffffda30      0x00000000      0xf7e33a63
0xffffda20:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda30:     0x00000002      0xffffdac4      0xffffdad0      0xf7feac7a
0xffffda40:     0x00000002      0xffffdac4      0xffffda64      0x080497fc
0xffffda50:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9f8:       0xffffda18
```
1. So, the most interesting thing above is the value in $ebp, as we haven't copied anything to memory yet
1. Let's continue, and see what happens when we start playing with memory
```
(gdb) c
Continuing.

Breakpoint 4, strcopy_is_the_devil (str=0xffffdc4c "AAA") at buffer-intro.c:18
/home/walsh/git/buffer-intro/buffer-intro.c:18:258:beg:0x8048473
After strcopy - 32 hex words starting at $esp
0xffffd9e0:     0x00000000      0x09ca212c      0x00000001      0x414141bd
0xffffd9f0:     0xffffdc00      0x0000002f      0xffffda18      0x08048499
0xffffda00:     0xffffdc4c      0xffffdac4      0xffffdad0      0xf7e4b39d
0xffffda10:     0xf7fc03c4      0xffffda30      0x00000000      0xf7e33a63
0xffffda20:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda30:     0x00000002      0xffffdac4      0xffffdad0      0xf7feac7a
0xffffda40:     0x00000002      0xffffdac4      0xffffda64      0x080497fc
0xffffda50:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9f8:       0xffffda18
```
