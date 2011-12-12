# stderred

stderr in red.

## About

stderred "monkey patches" write() function from libc in order to colorize all
stderr output that goes to terminal thus making it distinguishable from stdout.
Basically it wraps text that goes to file with descriptor "2" with proper
escape codes making text red.

It's implemented as a shared library and doesn't require recompilation of
existing binaries thanks to LD_PRELOAD feature of Linux dynamic linker.

## Instalation

### Compile

TODO

### Enable

    if [ -f $HOME/lib/stderred.so ] && [ -f $HOME/lib64/stderred.so ]; then
      export LD_PRELOAD=$HOME/\$LIB/stderred.so
    fi

(http://comments.gmane.org/gmane.comp.lib.glibc.user/868)

## Authors

Asheesh Laroia - original concept and initial implementation

Marcin Kulik - current implementation
