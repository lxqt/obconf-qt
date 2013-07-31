/*
    Copyright (C) 2013  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
    
    Part of the code in this file is taken from obconf:
    Copyright (c) 2003-2007   Dana Jansens
    Copyright (c) 2003        Tim Riley

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

using namespace Obconf;

MainDialog::MainDialog():
  QDialog() {

  ui.setupUi(this);
  // resize the list widget according to the width of its content.
  ui.listWidget->setMaximumWidth(ui.listWidget->sizeHintForColumn(0) + ui.listWidget->frameWidth() * 2 + 2);

  /* read the config flie */
  loadSettings();

  theme_setup_tab();
  appearance_setup_tab();
  windows_setup_tab();
  mouse_setup_tab();
  moveresize_setup_tab();
  margins_setup_tab();
  desktops_setup_tab();
  dock_setup_tab();

  /* connect signals and slots */
  QMetaObject::connectSlotsByName(this);
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
