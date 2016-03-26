#/bin/bash

BINARY_LOCATION="/foo/bar"

function main()
{
    cmake .
    make
    cp emulationstation "$BINARY_LOCATION"
}

main