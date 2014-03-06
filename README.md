# Mupen64Plus-Qt

A basic cross-platform launcher for Mupen64Plus. This was adapted from CEN64-Qt to work with Mupen64Plus.

## Building

### Linux

You'll need to make sure you have qmake, g++ and the Qt development libraries installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt4-qmake g++ libqt4-dev
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

```
$ qmake
$ make
```