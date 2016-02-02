#!/bin/bash

POTFILE="./emulationstation2.pot"

for POFILE in lang/*/LC_MESSAGES/emulationstation2.po
do
    msgmerge -U "${POFILE}" "${POTFILE}"
done

echo

for POFILE in lang/*/LC_MESSAGES/emulationstation2.po
do
    POLANG=$(echo "${POFILE}" | sed -e s+"lang/\([^/]*\)/.*$"+'\1'+)
    printf "%-5s: " "${POLANG}"
    msgfmt "${POFILE}" -o - --statistics >/dev/null
done
