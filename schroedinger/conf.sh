export CFLAGS="-O3 -g"
autoreconf -i -f && ./configure --prefix=/home/wladimir --with-cuda=/usr/local/cuda
