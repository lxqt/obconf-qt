/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "maindialog.h"
#include <string.h>
#include <math.h>
#include <alloca.h>
#include <stdlib.h>

// FIXME: Qt5 does not have QX11Info
#if QT_VERSION >= 0x050000
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow.h>
#else
#include <QX11Info>
#endif

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStringBuilder>

// FIXME: how to support XCB or Wayland?
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

using namespace Obconf;

MainDialog::MainDialog():
  QDialog() {

  ui.setupUi(this);

  /* read the config flie */
  loadSettings();

}

MainDialog::~MainDialog() {

}

void MainDialog::loadSettings() {

}

void MainDialog::accept() {
  QDialog::accept();
}

void MainDialog::reject() {
  /* restore to original settings */

  QDialog::reject();
}
