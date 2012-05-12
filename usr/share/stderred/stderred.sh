# Source this file in your shell to enable stderred for all commands

if [[ ! "$LD_PRELOAD" == *libstderred.so* ]]; then
  export LD_PRELOAD="/usr/lib/libstderred.so${LD_PRELOAD:+:$LD_PRELOAD}"
fi
