#!/bin/bash
set -e
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        --target)
            TARGET="$2"
            shift # past argument
            shift # past value
            ;;
        --config)
            BUILD_CONFIG="$2"
            shift # past argument
            shift # past value
            ;;
        *)
            POSITIONAL_ARGS+=("$1") # save positional arg
            shift # past argument
            ;;
    esac
done

# default values
if [ -z "$TARGET" ]; then
    TARGET="HP"
fi

if [ -z "$BUILD_CONFIG" ]; then
    BUILD_CONFIG="Debug"
fi

echo "Flashing firmware for ${TARGET}"

if [ "$TARGET" == "HE" ] || [ "$TARGET" == "HP" ] || [ "$TARGET" == "HP_SRAM" ] || [ "$TARGET" == "HE_DEVKIT" ] || [ "$TARGET" == "HP_DEVKIT" ] || [ "$TARGET" == "HP_SRAM_DEVKIT" ] || [ "$TARGET" == "E1C" ]; then
    cp ./out/firmware-alif/${TARGET}/${BUILD_CONFIG}/firmware-alif-${TARGET}.bin $SETOOLS_ROOT/build/images/alif-img.bin
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
