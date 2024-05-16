set -e
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "Building firmware"

cd $SCRIPTPATH
cbuild ./firmware-alif.csolution.yaml --context-set --update-rte --packs --context firmware-alif.debug+HP
