# ObConf-Qt

## Overview

ObConf-Qt is a Qt port of [ObConf](http://openbox.org/wiki/ObConf:About), a configuration editor for window manager [OpenBox](http://openbox.org).   

It is not actively developed anymore by LXQt project, code contributions and bugfixes will be accepted.
It can be used independently from this desktop environment.   

## Installation

### Compiling source code

Runtime dependencies are Qt X11 Extras, gtk-update-icon-cache, hicolor-icon-theme and Openbox.   
Additional build dependencies are CMake, [liblxqt](https://github.com/lxqt/liblxqt) and [lxqt-build-tools](https://github.com/lxqt/lxqt-build-tools), 
optionally Git to pull latest VCS checkouts.

Code configuration is handled by CMake. CMake variable `CMAKE_INSTALL_PREFIX` has to be set to `/usr` on most operating systems.   

To build run `make`, to install `make install` which accepts variable `DESTDIR` as usual.   

### Binary packages

Official binary packages are available in the most common distributions, search in your package manager for `obconf-qt`.

### Translations

Translations can be done in [LXQt-Weblate](https://translate.lxqt-project.org/projects/lxqt-configuration/obconf-qt/).

<a href="https://translate.lxqt-project.org/projects/lxqt-configuration/obconf-qt/">
<img src="https://translate.lxqt-project.org/widgets/lxqt-configuration/-/obconf-qt/multi-auto.svg" alt="Translation status" />
</a>
