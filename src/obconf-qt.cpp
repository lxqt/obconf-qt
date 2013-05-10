#include "obconf-qt.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include "maindialog.h"

using namespace Obconf;

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  // load translations
  QTranslator qtTranslator, translator;
  // install the translations built-into Qt itself
  qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  // install our own tranlations
  translator.load("obconf-qt_" + QLocale::system().name(), PACKAGE_DATA_DIR "/translations");
  app.installTranslator(&translator);

  // build our GUI
  MainDialog dlg;
  dlg.exec();
  return 0;
}
