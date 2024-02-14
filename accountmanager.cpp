#include "accountmanager.h"
#include "marketdatasingleton.h"

AccountManager::AccountManager()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", 0);
    connectSlots();
    client->connect();
    client->start();
}

AccountManager::AccountManager(TwsClientQThreaded* client)
{
    this->client = client;
}

void AccountManager::getNextValidOrderId()
{
    client->reqNextValidOrderId();
}

void AccountManager::subscribeAccountUpdates()
{
    client->reqAccountUpdates(client->accountNumber);
}

void AccountManager::subscribePortoflioUpdates()
{
    client->reqPositionUpdates(subscribePortfolioUpdates);
}

void AccountManager::onSignalLogger(QString msg)
{
    emit signalPassLogMsg(msg);
}

void AccountManager::onSignalNextValidId(OrderId id)
{
    auto msg = fmtlog(logger, "%s: next order id is %d", __func__, id);
    emit signalPassLogMsg(msg);

    if(!orderIdInitted)
    {
        orderId = id;
        orderIdInitted = true;
    }
}

void AccountManager::onSignalAccountUpdateValue(
    const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName)
{
    //emit signalPassLogMsg(QString("AccountManager %1 %2").arg(QString(key.c_str()), QString(val.c_str())));
    emit signalAccountUpdatesDraw(key, val, currency, accountName);
}

void AccountManager::onSignalUpdatePortfolio(const Contract& contract, Decimal position, double marketPrice, double marketValue,
                             double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName)
{
    //emit signalPassLogMsg(QString("AccountManager %1").arg(QString(contract.symbol.c_str())));
    emit signalPortfolioUpdatesDraw(contract, position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName);
    ActivePosition pos{contract, position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName};
    activePositions[contract.conId] = pos;
}

void AccountManager::onSignalOrderStatus(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
                         double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
                         double mktCapPrice)
{
    //emit signalPassLogMsg(QString("AccountManager orderStatus %1 %2").arg(QString::number(orderId), QString(status.c_str())));
    emit signalOrderStatusDraw(orderId, status, filled, remaining, avgFillPrice, permId, parentId, lastFillPrice, clientId, whyHeld, mktCapPrice);

    if(pendingOrders.find(orderId) == pendingOrders.end())
    {
        PendingOrderStruct pending;
        pending.status = status;
        pending.filled = filled;
        pending.remaining = remaining;
        pending.avgFillPrice = avgFillPrice;
        pending.permId = permId;
        pending.parentId = parentId;
        pending.lastFillPrice = lastFillPrice;
        pending.clientId = clientId;
        pending.whyHeld = whyHeld;
        pending.mktCapPrice = mktCapPrice;
        pendingOrders[orderId] = pending;
    }
    else
    {
        pendingOrders[orderId].status = status;
        pendingOrders[orderId].filled = filled;
        pendingOrders[orderId].remaining = remaining;
        pendingOrders[orderId].avgFillPrice = avgFillPrice;
        pendingOrders[orderId].permId = permId;
        pendingOrders[orderId].parentId = parentId;
        pendingOrders[orderId].lastFillPrice = lastFillPrice;
        pendingOrders[orderId].clientId = clientId;
        pendingOrders[orderId].whyHeld = whyHeld;
        pendingOrders[orderId].mktCapPrice = mktCapPrice;
    }
    //pendingOrders[orderId] should be present by now
    //if(pendingOrders[orderId].state.status == "Submitted") //Submitted is the status we want to use in production
    if(pendingOrders[orderId].status == "Submitted") //Submitted is the status we want to use in production
    {
        orderTimers[orderId] = new QTimer();
        connect(orderTimers[orderId], &QTimer::timeout, this, std::bind(&AccountManager::updateBid, this, orderId));
        orderTimers[orderId]->setSingleShot(true);
        orderTimers[orderId]->start(30e3); //ms
        auto msg = fmtlog(logger, "%s: starting order timer for orderID %d", __func__, orderId);
        emit signalPassLogMsg(msg);
    }
    //else if(pendingOrders[orderId].state.status == "Filled")
    else if(pendingOrders[orderId].status == "Filled")
    {
        if(orderTimers.find(orderId) == orderTimers.end())
        {
            auto msg = fmtlog(logger, "%s, WARN: Order ID %d filled but no timer to stop", __func__, orderId);
            emit signalPassLogMsg(msg);
            return;
        }
        orderTimers[orderId]->stop();
        auto msg = fmtlog(logger, "%s: stopping order timer for orderID %d", __func__, orderId);
        emit signalPassLogMsg(msg);
        emit signalUnSubscribeDataBrokerMktData(pendingOrders[orderId].contract);
    }
}

void AccountManager::onSignalOpenOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state)
{
    emit signalOpenOrderDraw(orderId, contract, order, state);
    if(pendingOrders.find(orderId) == pendingOrders.end())
    {
        PendingOrderStruct pending;
        pending.contract = contract;
        pending.order = order;
        pending.state = state;

        pendingOrders[orderId] = pending;
        auto msg = fmtlog(logger, "%s: orderId %d opened", __func__, orderId);
        emit signalPassLogMsg(msg);
        emit signalSubscribeDataBrokerMktData(contract);
    }
    else
    {
        pendingOrders[orderId].contract = contract;
        pendingOrders[orderId].order = order;
        pendingOrders[orderId].state = state;
        auto msg = fmtlog(logger, "%s: orderId %d updated", __func__, orderId);
        emit signalPassLogMsg(msg);
    }
}

Contract AccountManager::recreateContract(Contract con)
{
    Contract contract;
    contract.symbol = con.symbol;
    contract.secType = con.secType;
    contract.right = con.right;
    contract.strike = con.strike;
    contract.exchange = con.exchange;
    contract.lastTradeDateOrContractMonth = con.lastTradeDateOrContractMonth;
    return contract;
}

Order AccountManager::recreateOrder(Order ord)
{
    Order order;
    order.action = ord.action;
    order.orderType = ord.orderType;
    order.lmtPrice = ord.lmtPrice;
    order.totalQuantity = ord.totalQuantity;
    order.transmit = ord.transmit;
    order.algoStrategy = ord.algoStrategy;
    order.algoParams = ord.algoParams;
    return order;
}

void AccountManager::updateBid(OrderId orderId)
{
    auto msg = fmtlog(logger, "%s: updating orderid %d\n", __func__, orderId);
    emit signalPassLogMsg(msg);
    //looks like modifying order only works if you recreate the same order as before exactly the same
    auto databank = MarketDataSingleton::GetInstance();

    auto pendingContract = pendingOrders[orderId].contract;
    auto contract = recreateContract(pendingContract);
    auto pendingOrder = pendingOrders[orderId].order;
    auto order = recreateOrder(pendingOrder);
    /*Contract contract;
    contract.symbol = pendingContract.symbol;
    contract.secType = pendingContract.secType;
    contract.right = pendingContract.right;
    contract.strike = pendingContract.strike;
    contract.exchange = pendingContract.exchange;
    contract.lastTradeDateOrContractMonth = pendingContract.lastTradeDateOrContractMonth;
    Order order;
    order.action = pendingOrder.action;
    order.orderType = pendingOrder.orderType;

    double bid = databank->getMktData(optionContractString(contract), TickType::BID);
    double ask = databank->getMktData(optionContractString(contract), TickType::ASK);
    double mid = (bid + ask) / 2;
    order.lmtPrice = std::ceil(mid * 100.0) / 100.0;;
    order.totalQuantity = pendingOrder.totalQuantity;
    order.transmit = pendingOrder.transmit;
    order.algoStrategy = pendingOrder.algoStrategy;
    order.algoParams = pendingOrder.algoParams;*/

    double bid = databank->getMktData(optionContractString(contract), TickType::BID);
    double ask = databank->getMktData(optionContractString(contract), TickType::ASK);
    double mid = (bid + ask) / 2;
    order.lmtPrice = std::ceil(mid * 100.0) / 100.0;;

    //stop this instance of timer beacause a new timer will be created onSignalOrderStatus
    orderTimers[orderId]->stop();
    client->placeOrder(orderId, contract, order);
}

void AccountManager::onReceivePlaceOrder(Contract contract, Order order)
{
    if(contract.secType == "OPT")
    {
        auto msglog = fmtlog(logger, "%s: received to place order- OrderId: %d, Symbol: %s, SecType: %s"
            "%s %.2f %s Action %s, Type: %s, LmtPrice: %.2f, Qty: %s", __func__, orderId, contract.symbol.c_str(),
            contract.secType.c_str(), contract.lastTradeDateOrContractMonth.c_str(), contract.strike, contract.right.c_str(),
            order.action.c_str(), order.orderType.c_str(), order.lmtPrice, DecimalFunctions::decimalStringToDisplay(order.totalQuantity).c_str());
        emit signalPassLogMsg(msglog);

        if(contract.secType == "OPT")
        {
            for(const auto& pendingOrder : pendingOrders)
            {
                if(pendingOrder.second.contract.symbol == contract.symbol &&
                    pendingOrder.second.contract.secType == contract.secType &&
                    pendingOrder.second.contract.strike == contract.strike &&
                    pendingOrder.second.contract.right == contract.right &&
                    pendingOrder.second.contract.lastTradeDateOrContractMonth == contract.lastTradeDateOrContractMonth &&
                    pendingOrder.second.contract.exchange == contract.exchange)
                    {
                        QString msg = QString("conId %1 symbol %2 position already exists").arg(
                            QString(contract.secType.c_str()), QString(contract.symbol.c_str()));
                        emit signalPassLogMsg(msg);
                        return;
                    }
            }
        }

        submittedOrders[orderId] = OrderStruct{contract, order};
        client->placeOrder(orderId, contract, order);
        orderId += 1;
    }
}
