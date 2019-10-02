# Do not invoke directly
# use Makefile

CONFIG=$1
LINUX=$2
MAKE=$3

rm "${CONFIG}"

cd "${LINUX}"
"${MAKE}" x86_64_defconfig
"${MAKE}" kvmconfig

cd ..
patch "${CONFIG}" linux_debug_config.diff
