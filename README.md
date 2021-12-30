# The Synacor Challenge

The [Synacor Challenge][1] presents its participants
with a computer program and a description of the architecture
on which it runs.
The problem is ostensibly to construct a VM emulating this architecture
and run the provided code.
The `lib` and `svm` directories contain the components of this VM.

A suite of development tools is also included:
`synas` is an assembler, `synld` a linker,
and `svm` itself contains an integrated debugger
that activates upon receipt of a **SIGINT** or **SIGUSR1** signal.

## Building and Installing

This system was developed on macOS 12 using the NetBSD-derived `bmake`
from Homebrew.

```sh
$ brew install bmake
$ bmake && sudo bmake install
```

Other systems have not yet been tested.

## Design

The VM itself is fairly simple.
The machine state is initialized
and the program loaded into a dynamically expanding buffer
representing the system's RAM.
Reads or writes beyond the end of the array cause it to grow.
At each step, an instruction is read from memory
and the appropriate action taken.
Step until halt.

The development suite was more interesting.
After having used `lldb` on my host system
to inspect and manipulate the virtualized code,
it seemed only natural to implement a basic debugger
for the virtual system.
After implementing disassembly
and inspection and modification of RAM and registers,
I had what I felt I needed.
But then after having made a disassembler,
the system felt incomplete without the inverse.
Thus an assembler was born.
In order to be able to write programs across multiple files,
I also made a very basic linker.

This was my first time implementing a relocating linker.
No distinction is made between code and data,
and there is only a single section.
Object files store addresses
relative to the first address of that object
and contain a list of locations that need to be updated
at link time when this information is available.
Then the objects contain a list of symbols they export
and their base-relative locations,
as well as a list of (location, reference) pairs for unknown symbols.
This provides the linker with enough information
to combine object files.
The only operation currently supported is combining several objects
into an executable;
that is, there is no support for archives (libraries) at the moment.

[1]: https://challenge.synacor.com
