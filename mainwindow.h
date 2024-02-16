#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "earningsvolatilitystrat.h"
#include "accountmanager.h"
#include "logwindow.h"
#include "twsclientqthreaded.h"
#include "strategies.h"
#include "optionschainagent.h"
#include <QMainWindow>
#include "agentcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void logMsg(QString msg)
    {
        emit signalLogMsg(msg);
    }

    void closeEvent(QCloseEvent* event)
    {
        //logWindow->close();
        QMainWindow::closeEvent(event);
    }

    void runStrats();

signals:
    void signalLogMsg(QString msg);

public slots:
    void onSignalLogFromStrat(QString msg);

//GUI drawing
    void onSignalAccountUpdatesDraw(
        const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName);
    void onSignalPortfolioUpdatesDraw(const Contract& contract, Decimal position, double marketPrice, double marketValue,
                             double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName);
    void onSignalOrderStatusDraw(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
        double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
        double mktCapPrice);
    void onSignalOpenOrderDraw(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state);

private slots:
    void on_pushButton_clicked();

private:
    void connectSignalsAndSlots();
    void connectClientSignalsAndSlots();
    void setupPendingOrdersTable();
    void setupActivePostitionsTable();

    Ui::MainWindow* ui;
    QSharedPointer<LogWindow> logWindow;

    QSharedPointer<IBaseAgent> earningStrat;
    //QSharedPointer<IBaseAgent> accountManager;
    //QSharedPointer<IBaseAgent> optionsAgent;

    QSharedPointer<AgentController> accountAgentController;
    QSharedPointer<AgentController> optionsChainAgentController;
    QSharedPointer<AgentController> dataBrokerController;
    QSharedPointer<AgentController> earningsStratController;
    QSharedPointer<AgentController> trendFollowStratController;
};
#endif // MAINWINDOW_H
