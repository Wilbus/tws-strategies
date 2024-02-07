#include "accountmanager.h"

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
}

void AccountManager::onSignalOpenOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state)
{
    //emit signalPassLogMsg(QString("AccountManager openOrderResp %1 %2").arg(QString::number(orderId), QString(contract.symbol.c_str())));
    emit signalOpenOrderDraw(orderId, contract, order, state);
    if(pendingOrders.find(orderId) == pendingOrders.end())
    {
        PendingOrderStruct pending;
        pending.contract = contract;
        pending.order = order;
        pending.state = state;

        pendingOrders[orderId] = pending;
    }
    else
    {
        pendingOrders[orderId].contract = contract;
        pendingOrders[orderId].order = order;
        pendingOrders[orderId].state = state;
    }
}

void AccountManager::onReceivePlaceOrder(Contract contract, Order order)
{
    if(contract.secType == "OPT")
    {
        auto msglog = fmtlog(logger, "%s: received to place order- OrderId: %D, Symbol: %S, SecType: %S,"
            "%s %.02f %s Action %s, Type: %s, LmtPrice: %.02f, Qty: %s", __func__, orderId, contract.symbol.c_str(),
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

        client->placeOrder(orderId++, contract, order);
        //client->reqNextValidOrderId();
    }
}
