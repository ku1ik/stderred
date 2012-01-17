# stderred

stderr in red.

## About

stderred hooks on write() function from libc in order to colorize all
stderr output that goes to terminal thus making it distinguishable from stdout.
Basically it wraps text that goes to file with descriptor "2" with proper
escape codes making text red.

It's implemented as a shared library and doesn't require recompilation of
existing binaries thanks to "preload/insert" feature of dynamic linkers.

It's supported on Linux (with `LD_PRELOAD`), FreeBSD (also `LD_PRELOAD`) and
OSX (with `DYLD_INSERT_LIBRARIES`).

## Installation

Clone this repository:

    $ git clone git://github.com/sickill/stderred.git
    $ cd stderred

Important: In all cases below make sure that path to `stderred.so` is absolute!

### 32-bit Linux or FreeBSD

    $ make lib/stderred.so

Export `LD_PRELOAD` variable in your shell's config file by putting following
in your .bashrc/.zshrc:

    export LD_PRELOAD="/absolute/path/to/lib/stderred.so"

### 64-bit Linux or FreeBSD

    $ make lib64/stderred.so

Export `LD_PRELOAD` variable in your shell's config file by putting following
in your .bashrc/.zshrc:

    export LD_PRELOAD="/absolute/path/to/lib64/stderred.so"

On some Linux distros you can install 32-bit packages on 64-bit system.  Shared
libraries compiled for 64-bit doesn't work with 32-bit binaries though. It
happens that 64-bit binaries call 32-bit ones resulting in warning message
printed to terminal about not compatible `LD_PRELOAD` shared lib.

Fortunately Linux's dynamic linker has a feature called Dynamic String Token
(DST). It allows dynamic substitution of `$LIB` token in `LD_PRELOAD` variable
with "lib" or "lib64" respectively for 32 and 64-bit binaries when the binary
is being run. Thanks to that you can compile stderred for both architectures
and automatically use proper version of this shared library.

On 64-bit Fedora, for example, you need to install libc development headers for
both architectures:

    $ sudo yum install glibc-devel.i686 glibc-devel.x86_64

compile it like this:

    $ make both

and export `LD_PRELOAD` like this in your shell's config:

    export LD_PRELOAD="/path/to/stderred/\$LIB/stderred.so"

_\* Note that [there is no support for $LIB token on Ubuntu](http://comments.gmane.org/gmane.comp.lib.glibc.user/974)._

### OSX

    $ make both
    $ lipo -create lib/stderred.so lib64/stderred.so -output lib/stderred.dylib

Export `DYLD_INSERT_LIBRARIES` variable in your shell's config file by putting following
in your .bashrc/.zshrc:

    export DYLD_INSERT_LIBRARIES=/absolute/path/to/lib/stderred.dylib DYLD_FORCE_FLAT_NAMESPACE=1

**Note:** Installing on OS X will break the `open` command line utility. So
things like `mvim` and `open` itself will not work unless the application being
opened is already opened. It's because of flat namespace forced by
`DYLD_FORCE_FLAT_NAMESPACE` which is required by `DYLD_INSERT_LIBRARIES`.

### Aliasing

Alternative to enabling it globally via shell config is to create alias and
use it to selectively colorize stderr for the commands you run:

    $ alias stderred='LD_PRELOAD=/absolute/path/to/lib/stderred.so'
    $ stderred java lol

### Checking if it works

    $ python -c 'import os; print "Yo!"; os.write(2, "Jola\n\r")'

Jola should be in red dress.

![stderred in action](https://github.com/downloads/sickill/stderred/stderred.png)

## Configuration

### Custom color code

If you prefer other color or you want to use additional escape codes
(for bold/bright, italic, different background) you can export
`STDERRED_ESC_CODE` with desired escape code sequence.

Here's example for bold red:

    export STDERRED_ESC_CODE=`echo -e "\e[1;31m"`

## Alternative implementations

Simpler and much less reliable solution when using Zsh is to use named pipes
trick proposed on
[Gentoo Linux wiki](http://en.gentoo-wiki.com/wiki/Zsh#Colorize_STDERR).
It has some race condition/buffering issues and breaks on interactive commands
writing to stderr though.

## Authors

[Original concept](http://www.asheesh.org/note/software/stderred.html) and
[initial implementation](http://git.asheesh.org/?p=zzz/colorize-stderr.git;a=summary):

* Asheesh Laroia

Current implementation:

* Marcin Kulik
* Brian Tarricone

## License

You are free to use this program under the terms of the license found in
LICENSE file.
