# A Stack Buffer Overflow

### TODO
- [x] Add builds for 64 bit binary
- [x] Document the walkthrough for 32 bit
- [ ] Document the walkthrough for 64 bit
- [ ] Show the stack and memory layout

### Overview

* This is a simple stack buffer overflow demonstration.
* Protections have been explicitly disabled in the compiler.
* The majority of this work is a reimplementation of [https://vimeo.com/33106013](https://vimeo.com/33106013)

### Further Reading
* [DEP](http://en.wikipedia.org/wiki/Data_Execution_Prevention)
* [ASLR](http://en.wikipedia.org/wiki/Address_space_layout_randomization)
* [Endianness](http://en.wikipedia.org/wiki/Endianness)
* [Null-terminated strings](http://en.wikipedia.org/wiki/Null-terminated_string)
* [The best intro to GDB](http://www.dirac.org/linux/gdb/)


### Walkthrough

### Goal
* To execute `cold_code()` and `very_cold_code()`.
* Why? Well, `cold_code()` and `very_cold_code()` exist in the source file, but `main()` doesn't call them, making them
inaccessable. To be able to control memory gives us the ability to randomly jump to these functions

#### Setup
1. Satisify your build dependencies. We're compiling to x86, so we need to have 32-bit libraries
  1. Lots of people will be running x64, so you might need to hunt down packages that look like `lib32gcc1`
1. Build your `overflow` by running `./build`
1. Optional - edit `~/.gdbinit` to allow the loading of the repo `.gdbinit` with `add-auto-load-safe-path <path to repo>/.gdbinit`
1. Enter the debugger with `gdb -q overflow`

#### Here we go!
* List the code with `list`. This is available due to our debugging information provided by the `-g` flag when compiling
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
*  We now want to set a breakpoint at line 17, before the strcopy
```
(gdb) b 17
Breakpoint 1 at 0x8048461: file buffer-intro.c, line 17.
```
* Let's run an automated command when we hit this break point(prompts removed so you can cut and paste)
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
* We now want to set a breakpoint at line 18, after the strcopy
```
(gdb) b 18
Breakpoint 4 at 0x8048473: file buffer-intro.c, line 18.
```
* We want to run the same commands automatically after the strcopy
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
* Now that we have breakpoints set, let's run the binary with some input
```
(gdb) run AAA
Starting program: /home/walsh/git/buffer-intro/overflow AAA
```
* We've hit the breakpoint and the associated commands have been executed
```
Breakpoint 3, strcopy_is_the_devil (str=0xffffdc4c "AAA") at buffer-intro.c:17
/home/walsh/git/buffer-intro/buffer-intro.c:17:234:beg:0x8048461
Before strcopy - 32 hex words starting at $esp
0xffffd9e0:     0x00000000      0x09ca212c      0x00000001      0x080482bd
0xffffd9f0:     0xffffdc26      0x0000002f      0xffffda18      0x08048499 <- $ebp is the thrid word
0xffffda00:     0xffffdc4c      0xffffdac4      0xffffdad0      0xf7e4b39d
0xffffda10:     0xf7fc03c4      0xffffda30      0x00000000      0xf7e33a63
0xffffda20:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda30:     0x00000002      0xffffdac4      0xffffdad0      0xf7feac7a
0xffffda40:     0x00000002      0xffffdac4      0xffffda64      0x080497fc
0xffffda50:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9f8:       0xffffda18
```
* So, the most interesting thing above is the value in $ebp, as we haven't copied anything to memory yet
* Let's continue, and see what happens when we start playing with memory
```
(gdb) c
Continuing.

Breakpoint 4, strcopy_is_the_devil (str=0xffffdc4c "AAA") at buffer-intro.c:18
/home/walsh/git/buffer-intro/buffer-intro.c:18:258:beg:0x8048473
After strcopy - 32 hex words starting at $esp
0xffffd9e0:     0x00000000      0x09ca212c      0x00000001      0x414141bd <- AAA
0xffffd9f0:     0xffffdc00      0x0000002f      0xffffda18      0x08048499 <- <null> in the first word
0xffffda00:     0xffffdc4c      0xffffdac4      0xffffdad0      0xf7e4b39d
0xffffda10:     0xf7fc03c4      0xffffda30      0x00000000      0xf7e33a63
0xffffda20:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda30:     0x00000002      0xffffdac4      0xffffdad0      0xf7feac7a
0xffffda40:     0x00000002      0xffffdac4      0xffffda64      0x080497fc
0xffffda50:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9f8:       0xffffda18
```
* Looking at `0xffffd9e0:`, we see that the last word(4 bytes) is `0x414141bd`. We've found "AAA" in memory!
* What isn't apparent, is that we've already technically overflowed the buffer due [http://en.wikipedia.org/wiki/Null-terminated_string](null-terminated strings).
* The `00` that terminates the string is in the first word at `0xffffd9f0:`. We've overflowed, but nothing bad happened? There are a variety of reasons that this
could have happened. Most likley is the over-allocation for memory alignment(submit a pull-request if I'm wrong)
* To more clearly illustrate the null termination of the string, we will re-run with `AA` as an input
```
(gdb) run AA
The program being debugged has been started already.
<snip>
/home/walsh/git/buffer-intro/buffer-intro.c:18:258:beg:0x8048473
After strcopy - 32 hex words starting at $esp
0xffffd9e0:     0x00000000      0x09ca212c      0x00000001      0x004141bd <- <null>AA
0xffffd9f0:     0xffffdc27      0x0000002f      0xffffda18      0x08048499
<snip>
```
* Now we see the null byte along with "AA"
* Through all of this, we haven't touched the EPB register, so we shouldn't be expecting anything exceptionally bad to happen
* Maybe we should start pushing the bounds?
 * Now that we're going to push bounds, we should introdce alternate methods of getting input to GDB. Two handy languages for doing this are Python and Perl(because they have character multlipication)
```
(gdb) run $(python -c 'print "A" * 10')
or
(gdb) run $(perl -e 'print "A" x 10')
```
* Experiment with what you can get away with before the program starts having issues
```
(gdb) run $(python -c 'print "A" * 10')
Starting program: /home/walsh/git/buffer-intro/overflow $(python -c 'print "A" * 10')
<snip>
After strcopy - 32 hex words starting at $esp
0xffffd9d0:     0x00000000      0x09ca212c      0x00000001      0x414141bd
0xffffd9e0:     0x41414141      0x00414141      0xffffda08      0x08048499  <- we're right up to the edge of $ebp
0xffffd9f0:     0xffffdc45      0xffffdab4      0xffffdac0      0xf7e4b39d
0xffffda00:     0xf7fc03c4      0xffffda20      0x00000000      0xf7e33a63
0xffffda10:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda20:     0x00000002      0xffffdab4      0xffffdac0      0xf7feac7a
0xffffda30:     0x00000002      0xffffdab4      0xffffda54      0x080497fc
0xffffda40:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9e8:       0xffffda08

(gdb) c
Continuing.
Exiting cleanly.
[Inferior 1 (process 32258) exited with code 021]
```
* So, making the next step, let's send "A" * 11
```
(gdb) run $(python -c 'print "A" * 11')
Starting program: /home/walsh/git/buffer-intro/overflow $(python -c 'print "A" * 11')
<snip>
Breakpoint 4, strcopy_is_the_devil (str=0xffffdc44 'A' <repeats 11 times>) at buffer-intro.c:18
/home/walsh/git/buffer-intro/buffer-intro.c:18:258:beg:0x8048473
After strcopy - 32 hex words starting at $esp
0xffffd9d0:     0x00000000      0x09ca212c      0x00000001      0x414141bd
0xffffd9e0:     0x41414141      0x41414141      0xffffda00      0x08048499
0xffffd9f0:     0xffffdc44      0xffffdab4      0xffffdac0      0xf7e4b39d
0xffffda00:     0xf7fc03c4      0xffffda20      0x00000000      0xf7e33a63
0xffffda10:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda20:     0x00000002      0xffffdab4      0xffffdac0      0xf7feac7a
0xffffda30:     0x00000002      0xffffdab4      0xffffda54      0x080497fc
0xffffda40:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9e8:       0xffffda00

(gdb) c
Continuing.
Exiting cleanly.

Program received signal SIGSEGV, Segmentation fault.
0xfffffe33 in ?? ()
```
* Jackpot! We've over-written part of `$ebp` with the null byte that terminates our string
* Let's add 4 more bytes("A"s) and see where we are
```
(gdb) run $(python -c 'print "A" * 15')
The program being debugged has been started already.
Start it from the beginning? (y or n) y
Starting program: /home/walsh/git/buffer-intro/overflow $(python -c 'print "A" * 15')
<snip>
After strcopy - 32 hex words starting at $esp
0xffffd9d0:     0x00000000      0x09ca212c      0x00000001      0x414141bd
0xffffd9e0:     0x41414141      0x41414141      0x41414141      0x08048400
0xffffd9f0:     0xffffdc40      0xffffdab4      0xffffdac0      0xf7e4b39d
0xffffda00:     0xf7fc03c4      0xffffda20      0x00000000      0xf7e33a63
0xffffda10:     0x080484c0      0x00000000      0x00000000      0xf7e33a63
0xffffda20:     0x00000002      0xffffdab4      0xffffdac0      0xf7feac7a
0xffffda30:     0x00000002      0xffffdab4      0xffffda54      0x080497fc
0xffffda40:     0x0804820c      0xf7fc0000      0x00000000      0x00000000

Address and contents of $ebp: 0xffffd9e8:       0x41414141 <- We are writing into $ebp

```

