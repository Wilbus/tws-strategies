#include "mainwindow.h"
#include "logformatter.h"
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

    accountAgentController = QSharedPointer<AgentController>(new AgentController(StrategyAgents::AccountManagerAgent));
    optionsChainAgentController = QSharedPointer<AgentController>(new AgentController(StrategyAgents::OptionsChainRetreiver));

    connectSignalsAndSlots();
    connectClientSignalsAndSlots();

    accountAgentController->start();
    optionsChainAgentController->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignalsAndSlots()
{
    connect(this, SIGNAL(signalLogMsg(QString)), logWindow.get(), SLOT(onSignalLogMsg(QString)));
    connect(accountAgentController->getAgent(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));
    connect(optionsChainAgentController->getAgent(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));

    connect(accountAgentController->getAgent(), SIGNAL(signalAccountUpdatesDraw(std::string,std::string,std::string,std::string)),
        this, SLOT(onSignalAccountUpdatesDraw(std::string,std::string,std::string,std::string)));

    connect(accountAgentController->getAgent(), SIGNAL(signalOpenOrderDraw(OrderId,Contract,Order,OrderState)),
        this, SLOT(onSignalOpenOrderDraw(OrderId,Contract,Order,OrderState)));
    connect(accountAgentController->getAgent(), SIGNAL(signalOrderStatusDraw(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)),
        this, SLOT(onSignalOrderStatusDraw(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)));

    connect(accountAgentController->getAgent(), SIGNAL(signalPortfolioUpdatesDraw(Contract,Decimal,double,double,double,double,double,std::string)),
        this, SLOT(onSignalPortfolioUpdatesDraw(Contract,Decimal,double,double,double,double,double,std::string)));
}

void MainWindow::onSignalLogFromStrat(QString msg)
{
    LogFormatter logger;
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
            //earningStrat = QSharedPointer<EarningsVolatilityStrat>(new EarningsVolatilityStrat());
            earningsStratController = QSharedPointer<AgentController>(new AgentController(StrategyAgents::EarningsVolatilityAgent));

            /*connect(earningStrat.get(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));

            connect(earningStrat.get(), SIGNAL(signalSendOrderToAccountManager(Contract,Order)),
                accountAgentController->getAgent(), SLOT(onReceivePlaceOrder(Contract,Order)));

            connect(earningStrat.get(), SIGNAL(signalRequestOptionsChain(std::vector<Contract>)),
                optionsChainAgentController->getAgent(), SLOT(onSignalGetOptionsChain(std::vector<Contract>)));

            connect(optionsChainAgentController->getAgent(), SIGNAL(signalSendSuperContracts(std::vector<SuperContract>)),
                earningStrat.get(), SLOT(onRecieveOptionsChain(std::vector<SuperContract>)));*/

            connect(earningsStratController->getAgent(), SIGNAL(signalPassLogMsg(QString)), this, SLOT(onSignalLogFromStrat(QString)));

            connect(earningsStratController->getAgent(), SIGNAL(signalSendOrderToAccountManager(Contract,Order)),
                accountAgentController->getAgent(), SLOT(onReceivePlaceOrder(Contract,Order)));

            connect(earningsStratController->getAgent(), SIGNAL(signalRequestOptionsChain(std::vector<Contract>)),
                optionsChainAgentController->getAgent(), SLOT(onSignalGetOptionsChain(std::vector<Contract>)));

            connect(optionsChainAgentController->getAgent(), SIGNAL(signalSendSuperContracts(std::vector<SuperContract>)),
                earningsStratController->getAgent(), SLOT(onRecieveOptionsChain(std::vector<SuperContract>)));

            //earningStrat->runStrat();
            earningsStratController->start();
            break;
        }
        default:
        {
            break;
        }
    }
}

