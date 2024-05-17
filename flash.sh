#!/bin/bash
set -e
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

if [ -z "$SETOOLS_ROOT" ]; then
    echo "SETOOLS_ROOT is not set!"
    exit 1
fi

if [ -z "$TARGET" ]; then
    TARGET="HP"
fi


if [ "$TARGET" == "HE" ] || [ "$TARGET" == "HP" ]; then
    cp ./out/firmware-alif/${TARGET}/debug/firmware-alif-${TARGET}.bin $SETOOLS_ROOT/build/images/alif-img.bin
    cp ./.alif/m55-${TARGET}_cfg.json $SETOOLS_ROOT/alif-img.json

    cd $SETOOLS_ROOT
    ./app-gen-toc -f alif-img.json
    ./app-write-mram -p

    rm ./build/images/alif-img.bin
    rm ./alif-img.json;
else
    echo "Invalid target: $TARGET"
    exit 1
fi
