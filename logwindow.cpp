#include "logwindow.h"

#include "ui_logwindow.h"

LogWindow::LogWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::LogWindow)
{
    ui->setupUi(this);
}

LogWindow::~LogWindow()
{
    delete ui;
}
