#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyle>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>

int main(int argc, char* argv[])
{
    auto file = QFile("/datadisk0/sambashare0/coding/deps/QSS/ElegantDark.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    auto stream = QTextStream(&file);

    QApplication a(argc, argv);
    a.setStyleSheet(stream.readAll());

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages)
    {
        const QString baseName = "strategiesbotwidget_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
