#!/bin/sh

while true; do
    rm -f /tmp/es-restart /tmp/es-sysrestart /tmp/es-shutdown
    ./emulationstation "$@"
    [ -f /tmp/es-restart ] && continue
    if [ -f /tmp/es-sysrestart ]; then
        rm -f /tmp/es-sysrestart
        sudo reboot
        break
    fi
    if [ -f /tmp/es-shutdown ]; then
        rm -f /tmp/es-shutdown
        sudo poweroff
        break
    fi
    break
done
