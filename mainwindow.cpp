#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupPendingOrdersTable();
    setupActivePostitionsTable();

    logWindow = QSharedPointer<LogWindow>(new LogWindow);
    // logWindow->setAttribute(Qt::WA_DeleteOnClose);
    logWindow->show();

    accountManager = QSharedPointer<AccountManager>(new AccountManager());
    optionsAgent = QSharedPointer<OptionsChainAgent>(new OptionsChainAgent());

    connectSignalsAndSlots();
    connectClientSignalsAndSlots();

    accountManager->subscribeAccountUpdates();
    accountManager->subscribePortoflioUpdates();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignalsAndSlots()
{
    connect(this, SIGNAL(signalLogMsg(QString)), logWindow.get(), SLOT(onSignalLogMsg(QString)));
    connect(accountManager.get(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));
    connect(optionsAgent.get(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));

    connect(accountManager.get(), SIGNAL(signalAccountUpdatesDraw(std::string,std::string,std::string,std::string)),
        this, SLOT(onSignalAccountUpdatesDraw(std::string,std::string,std::string,std::string)));

    connect(accountManager.get(), SIGNAL(signalOpenOrderDraw(OrderId,Contract,Order,OrderState)),
        this, SLOT(onSignalOpenOrderDraw(OrderId,Contract,Order,OrderState)));
    connect(accountManager.get(), SIGNAL(signalOrderStatusDraw(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)),
        this, SLOT(onSignalOrderStatusDraw(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)));

    connect(accountManager.get(), SIGNAL(signalPortfolioUpdatesDraw(Contract,Decimal,double,double,double,double,double,std::string)),
        this, SLOT(onSignalPortfolioUpdatesDraw(Contract,Decimal,double,double,double,double,double,std::string)));
}

void MainWindow::onSignalLogFromStrat(QString msg)
{
    logMsg(msg);
}

void MainWindow::connectClientSignalsAndSlots()
{

}

void MainWindow::runStrats()
{
    earningStrat->runStrat();
}

void MainWindow::on_pushButton_clicked()
{
    auto strategiesListCombo = findChild<QComboBox*>("strategiesListCombo");
    switch(strategiesListCombo->currentIndex())
    {
        case StrategyAgents::EarningsVolatilityAgent:
        {
            earningStrat = QSharedPointer<EarningsVolatilityStrat>(new EarningsVolatilityStrat());

            connect(earningStrat.get(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));

            connect(earningStrat.get(), SIGNAL(signalSendOrderToAccountManager(Contract,Order)),
                accountManager.get(), SLOT(onReceivePlaceOrder(Contract,Order)));

            connect(earningStrat.get(), SIGNAL(signalRequestOptionsChain(std::vector<Contract>)),
                optionsAgent.get(), SLOT(onSignalGetOptionsChain(std::vector<Contract>)));

            connect(optionsAgent.get(), SIGNAL(signalSendSuperContracts(std::vector<SuperContract>)),
                earningStrat.get(), SLOT(onRecieveOptionsChain(std::vector<SuperContract>)));

            earningStrat->runStrat();
            break;
        }
        default:
        {
            break;
        }
    }
}

