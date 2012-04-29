./configure --disable-mencoder --disable-altivec --enable-big-endian --disable-sighandler --enable-static   --disable-networking --disable-vidix --disable-vcd --disable-bluray \
  --disable-dvdnav --disable-dvdread --disable-dvdread-internal --disable-libdvdcss-internal  --disable-cddb  --disable-unrarexec --disable-ftp --disable-pthreads --disable-w32threads \
  --disable-win32dll \
  --prefix=$DEVKITXENON/usr --cc="xenon-gcc" --as="xenon-as" --ar="xenon-ar" \
  --enable-cross-compile --extra-cflags="-m32 -fno-pic -mpowerpc64 -I$DEVKITXENON/usr/include/ -Wall -Ilibxenon_miss" \
  --extra-ldflags="-m32 -maltivec -fno-pic -mpowerpc64 -L$DEVKITXENON/usr/lib/ -L$DEVKITXENON/xenon/lib/32/ -lxenon -T$DEVKITXENON/app.lds -u read -u _start -u exc_base" 