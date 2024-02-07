#ifndef STRATEGIES_H
#define STRATEGIES_H

#include <QObject>
#include "twsclientqthreaded.h"

enum AgentClientIds
{
    AccountManagerClient = 0,
    OptionsChainClient,
    EarningsVolatilityStratClient,
    EarningsVolatilityMonitorClient,
};

enum StrategyAgents
{
    EarningsVolatilityAgent = 0,
    EarningsVolatilityMonitorAgent
};


class BaseAgent : public QObject
{

    Q_OBJECT

public:
    virtual void runStrat() = 0;
    EWrapperQThreaded* getEWrapper()
    {
        return client->qewrapper;
    }

    void connectSlots()
    {
        connect(client->qewrapper, SIGNAL(signalError(int,int,std::string,std::string)),
            this, SLOT(onSignalError(int,int,std::string,std::string)));

        connect(client->qewrapper, SIGNAL(signalLogger(QString)),
            this, SLOT(onSignalLogger(QString)));

        connect(client->qewrapper, SIGNAL(signalNextValidId(OrderId)),
            this, SLOT(onSignalNextValidId(OrderId)));

        connect(client->qewrapper, SIGNAL(signalAccountUpdateValue(std::string,std::string,std::string,std::string)),
            this, SLOT(onSignalAccountUpdateValue(std::string,std::string,std::string,std::string)));

        connect(client->qewrapper, SIGNAL(signalUpdatePortfolio(Contract,Decimal,double,double,double,double,double,std::string)),
            this, SLOT(onSignalUpdatePortfolio(Contract,Decimal,double,double,double,double,double,std::string)));

        connect(client->qewrapper, SIGNAL(signalOrderStatus(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)),
            this, SLOT(onSignalOrderStatus(OrderId,std::string,Decimal,Decimal,double,int,int,double,int,std::string,double)));

        connect(client->qewrapper, SIGNAL(signalOpenOrder(OrderId,Contract,Order,OrderState)),
            this, SLOT(onSignalOpenOrder(OrderId,Contract,Order,OrderState)));

        connect(client->qewrapper, SIGNAL(signalLogger(QString)),
        this, SLOT(onSignalLogger(QString)));

        connect(client->qewrapper, SIGNAL(signalNextValidId(OrderId)), this,
            SLOT(onSignalNextValidId(OrderId)));

        connect(client->qewrapper, SIGNAL(signalHistoricalDataBar(Bar)), this,
            SLOT(onSignalHistoricalDataBar(Bar)));

        connect(client->qewrapper, SIGNAL(signalHistoricalDataBarEnd(QString, QString)), this,
            SLOT(onSignalHistoricalDataBarEnd(QString, QString)));

        connect(client->qewrapper, SIGNAL(signalHistoricalDataBarEndData(int, std::vector<Bar>)), this,
            SLOT(onSignalHistoricalDataBarEndData(int, std::vector<Bar>)));

        connect(client->qewrapper, SIGNAL(signalScanResultsDone(std::vector<ContractDetails>)), this,
            SLOT(onSignalScanResultsDone(std::vector<ContractDetails>)));

        connect(client->qewrapper, SIGNAL(signalFundamentalData(int, std::string)), this,
            SLOT(onSignalFundamentalData(int, std::string)));

        connect(client->qewrapper, SIGNAL(signalTickByTickMid(int, time_t, double)), this,
            SLOT(onSignalTickByTickMid(int, time_t, double)));

        connect(client->qewrapper,SIGNAL(signalSecurityDefinitionOptionalParameter(int,std::string,int,std::string,std::string,std::set<std::string>,std::set<double>)),
            this, SLOT(onSignalSecurityDefinitionOptionalParameter(int,std::string,int,std::string,std::string,std::set<std::string>,std::set<double>)));

        connect(client->qewrapper,SIGNAL(signalSecurityDefinitionOptionalParameterEnd(int)),
            this, SLOT(onSignalSecurityDefinitionOptionalParameterEnd(int)));

        connect(client->qewrapper,SIGNAL(signalTickPrice(TickerId,TickType,double,TickAttrib)), this,
            SLOT(onSignalTickPrice(TickerId,TickType,double,TickAttrib)));

        connect(client->qewrapper,SIGNAL(signalMarketDataType(TickerId,int)), this,
            SLOT(onSignalMarketDataType(TickerId,int)));

        connect(client->qewrapper,SIGNAL(signalContractDetails(int,ContractDetails)), this,
            SLOT(onSignalContractDetails(int,ContractDetails)));

        connect(client->qewrapper,SIGNAL(signalContractDetailsEnd(int)), this,
            SLOT(onSignalContractDetailsEnd(int)));
    }

protected:
    TwsClientQThreaded* client;

signals:
    void signalPassLogMsg(QString msg);
    void signalSendOrderToAccountManager(Contract contract, Order order);

public slots:
    virtual void onSignalError(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson) {};
    virtual void onSignalLogger(QString str) {};
    virtual void onSignalNextValidId(OrderId id){};
    virtual void onSignalScanResultsDone(std::vector<ContractDetails> results){};
    virtual void onSignalHistoricalDataBar(Bar bar){}; // for charting clientid
    virtual void onSignalHistoricalDataUpdate(int id, Bar bar){};
    virtual void onSignalSelectedScanResultsDataBar(Bar bar){};
    virtual void onSignalHistoricalDataBarEnd(QString startDate, QString endDate){};
    virtual void onSignalHistoricalDataBarEndData(int reqId, std::vector<Bar> bars){}; // for charting clientid
    virtual void onSignalSelectedScanResultsDataEnd(QString startDate, QString endDate){};
    virtual void onSignalRealTimeBar(
        long time, double open, double high, double low, double close, Decimal volume, Decimal wap, int count){};
    virtual void onSignalTickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize,
        Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk){};
    virtual void onSignalTickByTickMid(int reqId, time_t time, double midPoint){};
    virtual void onSignalAccountUpdateValue(
        const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName){};
    virtual void onSignalUpdatePortfolio(const Contract& contract, Decimal position, double marketPrice, double marketValue,
        double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName){};
    virtual void onSignalPositionsMulti(int reqId, const std::string& account, const std::string& modelCode,
        const Contract& contract, Decimal pos, double avgCost){};
    virtual void onSignalPositionsMultiEnd(int reqId){};
    virtual void onSignalOrderStatus(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
        double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
        double mktCapPrice){};
    virtual void onSignalOpenOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state){};
    virtual void onSignalContractDetails(int reqid, ContractDetails details){};
    virtual void onSignalContractDetailsEnd(int reqid){};
    virtual void onSignalWshMetaData(int reqId, const std::string& dataJson){};
    virtual void onSignalWshEventData(int reqId, const std::string& dataJson){};
    virtual void onSignalFundamentalData(int reqId, const std::string& dataXml){};
    virtual void onSignalSecurityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId,
        const std::string& tradingClass, const std::string& multiplier, const std::set<std::string>& expirations,
        const std::set<double>& strikes){};
    virtual void onSignalSecurityDefinitionOptionalParameterEnd(int reqId){};
    virtual void onSignalTickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib){};
    virtual void onSignalMarketDataType(TickerId reqId, int marketDataType){};
};

#endif // STRATEGIES_H
