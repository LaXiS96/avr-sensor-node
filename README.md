# AVR sensor node

ATtiny85-based low-power wireless sensor node. Uses BME280 humidity sensor and nRF24L01+ RF transceivers.

# Environment setup
Source: https://www.nongnu.org/avr-libc/user-manual/install_tools.html

```bash
sudo apt install build-essential texinfo libusb-1.0-0-dev autoconf libtool bison flex # tested on Ubuntu 20.04 and 20.10
PREFIX=$HOME/avr export PREFIX
PATH=$PREFIX/bin:$PATH export PATH
mkdir $PREFIX/build

# binutils
cd $PREFIX/build
wget https://ftp.gnu.org/gnu/binutils/binutils-2.35.1.tar.xz
tar xf binutils* && cd binutils-2.35.1
mkdir build && cd build
../configure --prefix=$PREFIX --target=avr --disable-nls
make
make install

# gcc
cd $PREFIX/build
wget ftp://ftp.lip6.fr/pub/gcc/releases/gcc-10.2.0/gcc-10.2.0.tar.xz
tar xf gcc* && cd gcc-10.2.0
./contrib/download_prerequisites
mkdir build && cd build
../configure --prefix=$PREFIX --target=avr --enable-languages=c --disable-nls --disable-libssp --with-dwarf2
make -j9 # number of cores/threads + 1
make install

# avr-libc
cd $PREFIX/build
wget http://download.savannah.gnu.org/releases/avr-libc/avr-libc-2.0.0.tar.bz2
tar xf avr-libc* && cd avr-libc-2.0.0
./configure --prefix=$PREFIX --build=`./config.guess` --host=avr
make
make install

# avrdude
cd $PREFIX/build
wget http://download.savannah.gnu.org/releases/avrdude/avrdude-6.3.tar.gz
tar xf avrdude* && cd avrdude-6.3
autoreconf -fi # to fix configure warning
mkdir build && cd build
../configure --prefix=$PREFIX
make
make install

# gdb
cd $PREFIX/build
wget https://ftp.gnu.org/gnu/gdb/gdb-10.1.tar.xz
tar xf gdb* && cd gdb-10.1
mkdir build && cd build
../configure --prefix=$PREFIX --target=avr
make
make install
```

# Programming

## ISP ATtiny85
Vcc -> 8
GND -> 4
RESET -> 1
MISO -> 6
MOSI -> 5
SCK -> 7

# Utils
https://www.engbedded.com/fusecalc/
