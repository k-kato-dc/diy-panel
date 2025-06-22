#!/bin/sh

echo "---- check required library ..."
LIB_NEEDED=0

PACKAGE_NAME="gcc"
rpm -qa | egrep "^${PACKAGE_NAME}"
if [ $? -ne 0 ]; then
  echo "Package not found: ${PACKAGE_NAME}"
  LIB_NEEDED=`expr ${LIB_NEEDED} + 1`
fi

PACKAGE_NAME="libusbx-devel"
rpm -qa | egrep "^${PACKAGE_NAME}"
if [ $? -ne 0 ]; then
  echo "Package not found: ${PACKAGE_NAME}"
  LIB_NEEDED=`expr ${LIB_NEEDED} + 1`
fi

if [ ${LIB_NEEDED} -ne 0 ]; then
  echo "need to add ${LIB_NEEDED} package(s)."
  exit 1
fi

MODULE_NAME="diy-led"
echo "---- build ${MODULE_NAME} ..."

if [ -e "${MODULE_NAME}" ]; then
	rm -f "$MODULE_NAME"
fi

gcc -L/usr/lib64 -O2 -o "${MODULE_NAME}" "${MODULE_NAME}.c"

exit 0
