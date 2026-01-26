#!/bin/bash
set -e
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

POSITIONAL_ARGS=()
CLEAN=0

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
        --clean)
            CLEAN=1
            shift # past argument
            ;;
        *)
            POSITIONAL_ARGS+=("$1") # save positional arg
            shift # past argument
            ;;
    esac
done

if [ "$CLEAN" == 1 ]; then
    echo "Cleaning"
    rm -rf tmp
    rm -rf out

    rm -f firmware-alif.cbuild*.yml
    rm -f firmware-alif.*.cprj
    rm -f *.cbuild.yml
    exit 0
fi

# default values
if [ -z "$TARGET" ]; then
    TARGET="HP"
fi

if [ -z "$BUILD_CONFIG" ]; then
    BUILD_CONFIG="Debug"
fi

echo "make: using ${MAKE_JOB} jobs"

if [ "$TARGET" == "HE" ] || [ "$TARGET" == "HP" ] || [ "$TARGET" == "HP_SRAM" ] || [ "$TARGET" == "HE_DEVKIT" ] || [ "$TARGET" == "HP_DEVKIT" ] || [ "$TARGET" == "HP_SRAM_DEVKIT" ] || [ "$TARGET" == "E1C" ]; then
    echo "Building firmware for ${TARGET}"
    cbuild ./firmware-alif.csolution.yml -j ${MAKE_JOBS:-$(nproc)} --context-set --update-rte --packs --context firmware-alif.${BUILD_CONFIG}+${TARGET} --toolchain GCC
else
    echo "Invalid target: $TARGET"
    exit 1
fi
