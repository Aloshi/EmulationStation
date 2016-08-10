#/bin/bash

BINARY_LOCATION="/foo/bar"
SOURCE_LOCATION="/other/foo/bar"

function main()
{
    cd "$SOURCE_LOCATION"
    cmake .
    make
    cp emulationstation "$BINARY_LOCATION"
}

main