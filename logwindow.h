#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include "qplaintextedit.h"

#include <QDateTime>
#include <QMainWindow>
#include <QTimeZone>

namespace Ui {
class LogWindow;
}

class LogWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget* parent = nullptr);
    ~LogWindow();

public slots:
    void onSignalLogMsg(QString msg)
    {
        auto logbox = findChild<QPlainTextEdit*>("logBox");
        //QString msgFmt = QDateTime::currentDateTime(QTimeZone::LocalTime).toString(Qt::ISODateWithMs) + ": " + msg;
        //logbox->appendPlainText(msgFmt);
        logbox->appendPlainText(msg);
    }

private:
    Ui::LogWindow* ui;
};

#endif // LOGWINDOW_H
