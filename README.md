# Mupen64Plus-Qt

A basic cross-platform launcher. This was adapted from CEN64-Qt to work with Mupen64Plus.

A discussion thread as well as links to binary downloads can be found [here](http://www.emutalk.net/threads/54976-Mupen64Plus-Qt).

## Building

### Linux

You'll need to make sure you have qmake, g++, the Qt development libraries and the Quazip development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt4-qmake g++ libqt4-dev libquazip-dev
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

```
$ qmake
$ make
```

## Usage

Mupen64Plus-Qt utilizes the console UI to launch games. It contains support for most of the command line parameters. These can be viewed by running Mupen64Plus from a terminal with the --help parameter or [here](https://code.google.com/p/mupen64plus/wiki/UIConsoleUsage).

### --noosd/--osd

One or the other is sent to Mupen64Plus based on whether the "On Screen Display" checkbox is checked under the graphics tab.

### --fullscreen/--windowed

Similarly, one or the other is sent based on the "Fullscreen" checkbox underneath the graphics tab.

### --resolution

This can be configured with the "Resolution" combo box underneath the graphics tab. The list is generated based on common resolutions smaller than your current desktop resolution. If you require a resolution not in the list, just set the "default" option which will not send the --resolution parameter to Mupen64Plus (Mupen64Plus will read from whatever setting is within it's configuration file in this case).

### --configdir

This is configured under the paths tab. This is entirely optional, but allows you to specific a different directory to load the mupen64plus.cfg file from (useful if you want to say, load different input settings at different times).

### --datadir

This is set underneath the paths tab. Mupen64Plus-Qt will also make use of the data directory setting to load the GoodName text and other information from the mupen64plus.ini file, so make sure this directory contains that file. It's usually /usr/share/mupen64plus on Linux or the same directory as the executable on Windows. 

### --plugindir

The plugin directory is usually /usr/lib/mupen64plus or within the same directory as the executable on Windows. Mupen64Plus-Qt also makes use of this directory setting to populate the options on the plugins tab (you must click Ok first before they will populate).

### --gfx/--audio/--input/--rsp

These are configured under the plugins tab once the plugin directory has been set.

### ---emumode

This is configured under the emulation tab.

### --saveoptions/--nosaveoptions

One or the other is sent to Mupen64Plus based on the "Save Options" checkbox underneath the other tab. If --saveoptions is set, then Mupen64Plus will save whatever settings are passed to it into its configuration file (mupen64plus.cfg)


## Game Information

Mupen64Plus-Qt supports downloading game information and cover images from [thegamesdb.net](http://thegamesdb.net/). This can be enabled under the Other tab. It's recommended you have the data directory set (under Paths) before using this. Once enabled, you'll need to refresh your ROMs list to download the information. Afterwards, images and other information about the game can be added to the layouts.

If a game is not found, Mupen64Plus-Qt supports refreshing information for a single ROM. Just select the rom and go to File->Download/Update Info. From here you can enter a different search or the specific ID of the game (from the URL of the game on thegamesdb.net).