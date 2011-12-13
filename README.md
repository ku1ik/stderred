# stderred

stderr in red.

## About

stderred "monkey patches" write() function from libc in order to colorize all
stderr output that goes to terminal thus making it distinguishable from stdout.
Basically it wraps text that goes to file with descriptor "2" with proper
escape codes making text red.

It's implemented as a shared library and doesn't require recompilation of
existing binaries thanks to LD_PRELOAD feature of Linux dynamic linker.

## Installation

Clone this repository:

    git clone git://github.com/sickill/stderred.git
    cd stderred

### Compiling

For 32-bit system:

    make lib/stderred.so

For 64-bit system:

    make lib64/stderred.so

### Enabling

You can enable stderred in 2 ways.

Recommended one is to export LD_PRELOAD variable in your shell's config file.
Put following in you .bashrc/.zshrc:

    if [ -f "/absolute/path/to/lib/stderred.so" ]; then
      export LD_PRELOAD="/absolute/path/to/lib/stderred.so"
    fi

Second option is to create alias and then use it to selectively colorize stderr
for run commands:

    $ alias stderred='LD_PRELOAD=/absolute/path/to/lib/stderred.so'
    $ stderred java lol

### Checking if it works

    $ python -c 'import os; print "Yo!"; os.write(2, "Jola\n\r")'

Jola should be in red dress.

### Multi-arch distros

On most Linux distros you can run 32-bit binaries on 64-bit system.  Shared
libraries compiled for 64-bit doesn't work with 32-bit binaries though. It
happens that 64-bit binaries call 32-bit ones resulting in warning message
printed to terminal about not compatible LD_PRELOAD shared lib.

Fortunately Linux's dynamic linker has a feature called Dynamic String Token
(DST). It allows dynamic substitution of `$LIB` token in LD_PRELOAD variable
with "lib" or "lib64" respectively for 32 and 64-bit binaries when the binary
is being run.

Thanks to that you can compile stderred for both architectures and
automatically use proper version of this shared library.

On Fedora, for example, you need to install libc development headers for both
architectures:

    sudo yum install libc-devel.i686 libc-devel.x86-64

compile it like this:

    make both

and export LD_PRELOAD like this:

    export LD_PRELOAD="/path/to/stderred/\$LIB/stderred.so"


Ubuntu doesn't support DST (http://comments.gmane.org/gmane.comp.lib.glibc.user/868)

## Authors

Asheesh Laroia - original concept and initial implementation

Marcin Kulik - current implementation

## License

You are free to use this program under the terms of the license found in
LICENSE file.
