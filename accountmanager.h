#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include "twsclientqthreaded.h"
#include "ReqIdTypes.h"
#include "strategies.h"
#include <QTimer>

class AccountManager : public BaseAgent
{

    Q_OBJECT
public:
    AccountManager();

    AccountManager(TwsClientQThreaded* client);

    void runStrat() override//this agent doesn't run strats
    {
        subscribeAccountUpdates();
        subscribePortoflioUpdates();
    }

private:
    void getNextValidOrderId();
    void subscribeAccountUpdates();
    void subscribePortoflioUpdates();
    Contract recreateContract(Contract contract);
    Order recreateOrder(Order order);

    //TwsClientQThreaded* client;
    OrderId orderId;
    bool orderIdInitted{false};
    OrderId sendOrderId;
    int subscribePortfolioUpdates{101};
    std::map<uint64_t, ActivePosition> activePositions;
    std::map<OrderId, PendingOrderStruct> pendingOrders;
    std::map<OrderId, OrderStruct> submittedOrders;
    std::map<OrderId, QTimer*> orderTimers;
    //std::map<OrderId, ReorderTimer> pendingOrderTimers;
    //std::map<OrderId, QSharedPointer<QTimer>> pendingOrderTimers;

signals:
//signals to send to gui window
    void signalAccountUpdatesDraw(
        const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName);
    void signalPortfolioUpdatesDraw(const Contract& contract, Decimal position, double marketPrice, double marketValue,
        double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName);
    void signalOrderStatusDraw(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
        double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
        double mktCapPrice);
    void signalOpenOrderDraw(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state);

//signals to send to monitoring agents

public slots:
//signals to receive from ewrapper
    void onSignalLogger(QString msg) override;
    void onSignalNextValidId(OrderId id) override;
    void onSignalAccountUpdateValue(
        const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName) override;
    void onSignalUpdatePortfolio(const Contract& contract, Decimal position, double marketPrice, double marketValue,
        double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName) override;
    void onSignalOrderStatus(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
        double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
        double mktCapPrice) override;
    void onSignalOpenOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state) override;

//signals to receieve to perform actions like placing orders
    void onReceivePlaceOrder(Contract contract, Order order);

//update bid price if it hasn't been filled within some time
    void updateBid(OrderId orderId);
};

#endif // ACCOUNTMANAGER_H
