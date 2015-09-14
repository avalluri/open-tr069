if [ ! -d m4 ]; then mkdir m4; fi
aclocal -I m4 &&
  autoheader &&
  libtoolize --copy --force &&
    autoconf &&
      automake --add-missing --copy

