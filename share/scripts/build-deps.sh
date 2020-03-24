#! /bin/bash

CFITSIO_VER=3390
CHEALPIX_VER=3.30.0

uid=$(id -u)
gid=$(id -g)


mkdir -p include lib


SCRIPT=$(cat <<-END
set -xe

cp /pwd/deps/cfitsio${CFITSIO_VER}.tar.gz .
tar -xzf cfitsio${CFITSIO_VER}.tar.gz
pushd cfitsio
./configure --prefix=/pwd
make shared
make install
popd
rm -rf /pwd/lib/libcfitsio.a /pwd/lib/pkgconfig
chown -h ${uid}:${gid} /pwd/lib/libcfitsio*
pushd /pwd/include
chown ${uid}:${gid} drvrsmem.h fitsio2.h fitsio.h longnam.h
popd

cp /pwd/deps/chealpix-${CHEALPIX_VER}.tar.gz .
tar -xzf chealpix-${CHEALPIX_VER}.tar.gz
cd chealpix-${CHEALPIX_VER}
gcc -shared -fPIC -O3 chealpix.c -o libchealpix.so.0 -I/pwd/include -L/pwd/lib -Wl,-rpath,'\$ORIGIN' -lcfitsio -lm
cp libchealpix.so.0 /pwd/lib
cp chealpix.h /pwd/include
cd ..
chown ${uid}:${gid} /pwd/lib/libchealpix.so.0 /pwd/include/chealpix.h
END
)


docker run --mount type=bind,source=$(pwd),target=/pwd                         \
           quay.io/pypa/manylinux1_x86_64 /bin/bash -c "${SCRIPT}"

[ -e lib/libchealpix.so.0 ]  && ln -fs libchealpix.so.0 lib/libchealpix.so
