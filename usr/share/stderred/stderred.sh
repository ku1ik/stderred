# Source this file in your shell to enable stderred for all commands

if [[ ! "$LD_PRELOAD" == *libstderred.so* ]]; then
  if [[ -f /usr/lib/libstderred.so ]]; then
    export LD_PRELOAD="/usr/lib/libstderred.so${LD_PRELOAD:+:$LD_PRELOAD}"
  elif [[ -f /usr/lib64/libstderred.so ]]; then
    export LD_PRELOAD="/usr/lib64/libstderred.so${LD_PRELOAD:+:$LD_PRELOAD}"
  fi
fi
