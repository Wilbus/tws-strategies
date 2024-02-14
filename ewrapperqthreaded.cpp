#include "ewrapperqthreaded.h"

#include <QCoreApplication>
#include <QList>
#include <iostream>
#include <string>

EWrapperQThreaded::EWrapperQThreaded(int clientid)
{
    clientId = clientid;
}

// Receive and display the current time
void EWrapperQThreaded::currentTime(long curTime)
{
    epochBuff = curTime;
    QString timeStr = asctime(localtime(&epochBuff));
}

// Respond to errors
void EWrapperQThreaded::error(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson)
{
    if (!advancedOrderRejectJson.empty())
    {
        auto msglog = fmtlog(logger, "%s: %d, code: %d, Msg: %s, RejectJson: %s",
            __func__, id, code, msg.c_str(), advancedOrderRejectJson.c_str());
        emit signalLogger(msglog);
    }
    else
    {
        auto logmsg = fmtlog(logger, "%s: Id: %d, code: %d, Msg: %s", __func__, id, code, msg.c_str());
        emit signalLogger(logmsg);
    }
    emit signalError(id, code, msg, advancedOrderRejectJson);
}

// Receives symbols for contracts related to the given string
void EWrapperQThreaded::symbolSamples(int reqId, const std::vector<ContractDescription>& descs)
{
    // std::cout << "Number of descriptions: " << descs.size() << std::endl;

    contractDescriptionsBuff.clear();

    for (ContractDescription desc : descs)
    {
        // std::cout << "Symbol: " << desc.contract.symbol << std::endl;
        contractDescriptionsBuff.push_back(desc);
    }
}

// Receives details related to the contract of interest
void EWrapperQThreaded::contractDetails(int reqId, const ContractDetails& details)
{
    std::cout << "Received ContractDetail Long name: " << details.longName << std::endl;
    // std::cout << "Category: " << details.category << std::endl;
    // std::cout << "Subcategory: " << details.subcategory << std::endl;
    // std::cout << "Contract ID: " << details.contract.conId << std::endl;
    contractDetailsBuff = details;
    emit signalContractDetails(reqId, details);
}

// Called when all contract data has been received
void EWrapperQThreaded::contractDetailsEnd(int reqId)
{
    std::cout << "ContractDetails done" << std::endl;
    emit signalContractDetailsEnd(reqId);
}

// Provide the ID of the next order
void EWrapperQThreaded::nextValidId(OrderId id)
{
    orderIdBuff = id;
    auto msg = fmtlog(logger, "%s: next valid ID %d", __func__, id);
    emit signalLogger(msg);
    emit signalNextValidId(id);
}

// Respond when the order is placed
void EWrapperQThreaded::openOrder(
    OrderId orderId, const Contract& contract, const Order& order, const OrderState& state)
{
    auto msg = fmtlog(logger, "%s: Order ID: %d, Contract: %s, Price %.02f, Qty: %s, Type: %s",
        __func__, orderId, contract.symbol.c_str(), order.lmtPrice,
        DecimalFunctions::decimalToString(order.totalQuantity).c_str(),
        order.orderType.c_str());
    emit signalLogger(msg);
    emit signalOpenOrder(orderId, contract, order, state);
}

// Provide the order's status when an order is placed
void EWrapperQThreaded::orderStatus(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
    double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
    double mktCapPrice)
{
    auto msg = fmtlog(logger, "%s: Order ID: %d, status: %s", __func__, orderId, status.c_str());
    emit signalLogger(msg);
    emit signalOrderStatus(orderId, status, filled, remaining, avgFillPrice, permId, parentId, lastFillPrice, clientId,
        whyHeld, mktCapPrice);
}

// Provide data related to the account's open positions
// reqPositions()
void EWrapperQThreaded::position(const std::string& account, const Contract& contract, Decimal pos, double avgCost)
{
    std::cout << "Position in " << contract.symbol << ": " << pos << std::endl;
}

// reqPositions()
void EWrapperQThreaded::positionEnd()
{
    std::cout << "position end\n";
}

// Provide data related to the account
// reqAccountSummary()
void EWrapperQThreaded::accountSummary(int reqId, const std::string& account, const std::string& tag,
    const std::string& value, const std::string& currency)
{
    // log("reqId: %1 account: %2, tag: %3, value: %4, currency: %5\n", qreqId, qaccount, qtag,
    //     qvalue, qcurrency);
}
// reqAccountSummary()
void EWrapperQThreaded::accountSummaryEnd(int reqId)
{
    // log("reqId: %1, accountSummaryEnd", reqId);
}

void EWrapperQThreaded::positionMulti(int reqId, const std::string& account, const std::string& modelCode,
    const Contract& contract, Decimal pos, double avgCost)
{
    emit signalPositionsMulti(reqId, account, modelCode, contract, pos, avgCost);
}

void EWrapperQThreaded::positionMultiEnd(int reqId)
{
    emit signalPositionsMultiEnd(reqId);
}

// reqAccountUpdates()
void EWrapperQThreaded::updateAccountValue(
    const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName)
{
    // std::printf("key: %s, val: %s, currency %s: accountName: %s\n", key.c_str(), val.c_str(), currency.c_str(),
    //     accountName.c_str());
    emit signalAccountUpdateValue(key, val, currency, accountName);
}
// reqAccountUpdates()
void EWrapperQThreaded::updatePortfolio(const Contract& contract, Decimal position, double marketPrice,
    double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName)
{
    std::printf("updatePortfolio callback\n");
    emit signalUpdatePortfolio(
        contract, position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName);
}
// reqAccountUpdates()
void EWrapperQThreaded::updateAccountTime(const std::string& timeStamp)
{
    std::printf("updateAccountTime callback: %s\n", timeStamp.c_str());
}
// reqAccountUpdates()
void EWrapperQThreaded::accountDownloadEnd(const std::string& accountName)
{
    std::printf("accountDownloadEnd: %s\n", accountName.c_str());
}

// Called in response to reqTickByTickData
void EWrapperQThreaded::tickByTickMidPoint(int reqId, time_t time, double midPoint)
{
    // std::cout << "tickByTickMidPoint - Midpoint tick: " << midPoint << std::endl;
    emit signalTickByTickMid(reqId, time, midPoint);
}

// Called in response to reqTickByTickData
void EWrapperQThreaded::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size,
    const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions)
{
    std::cout << "tickByTickAllLast - Last tick: " << price << std::endl;
}

// Called in response to reqTickByTickData
void EWrapperQThreaded::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize,
    Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk)
{
    // std::cout << "tickByTickBidAsk - Bid,Ask tick: " << bidPrice << ", " << askPrice << std::endl;
    emit signalTickByTickBidAsk(reqId, time, bidPrice, askPrice, bidSize, askSize, tickAttribBidAsk);
}

// Called in response to reqMktData
// TickType is an enum in the EWrapper class
void EWrapperQThreaded::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib)
{
    //std::cout << "tickPrice - field: " << field << ", price: " << price << std::endl;
    emit signalTickPrice(tickerId, field, price, attrib);
}

// Called in response to reqMktData
void EWrapperQThreaded::tickSize(TickerId tickerId, TickType field, Decimal size)
{
    std::cout << "tickSize - field: " << field << ", size: " << size << std::endl;
}

// Called in response to reqMktData
void EWrapperQThreaded::tickOptionComputation(TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol,
    double delta, double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice)
{
    std::cout << "tickOptionComputation - optPrice: " << optPrice << std::endl;
}

// Called in response to reqRealTimeBars
void EWrapperQThreaded::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
    Decimal volume, Decimal wap, int count)
{
    //QString msg = QString("historical bar update time: %1\n").arg(time);
    //emit signalLogger(msg);
    emit signalRealTimeBar(time, open, high, low, close, volume, wap, count);
}

// Called in response to reqHistoricalData
// TickerId is really just the reqId number
/*date timestamp returned in format "yyyyMMdd HH:mm:ss*/
void EWrapperQThreaded::historicalData(TickerId reqId, const Bar& bar)
{
    historicalDataBuff.push_back(bar);
    emit signalHistoricalDataBar(bar);
}

void EWrapperQThreaded::historicalDataUpdate(TickerId reqId, const Bar& bar)
{
    emit signalHistoricalDataUpdate(reqId, bar);
}

void EWrapperQThreaded::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr)
{
    // QString msg = QString("historical data end\n");
    // emit signalLogger(msg);
    emit signalHistoricalDataBarEndData(reqId, historicalDataBuff);
    historicalDataBuff.clear();
    emit signalHistoricalDataBarEnd(QString(startDateStr.c_str()), QString(endDateStr.c_str()));
}

// Called in response to reqFundamentalData
void EWrapperQThreaded::fundamentalData(TickerId reqId, const std::string& data)
{
    // std::cout << "Fundamental data: " << data << std::endl;
    emit signalFundamentalData(reqId, data);
}

// Obtain contract ID
void EWrapperQThreaded::scannerData(int reqId, int rank, const ContractDetails& details, const std::string& distance,
    const std::string& benchmark, const std::string& proj, const std::string& legsStr)
{
    //std::cout << rank << ": " << details.contract.symbol << std::endl;
    contractScanResultsBuff.push_back(details);
}

void EWrapperQThreaded::scannerDataEnd(int reqId)
{
    // std::cout << "Number of results: " << contractScanResultsBuff.size() << std::endl;
    // QString msg = QString("Number of results") + QString::number(contractScanResultsBuff.size());
    // emit signalLogger(msg);
    emit signalScanResultsDone(contractScanResultsBuff);
    contractScanResultsBuff.clear();
    //std::printf("emitted signalScanResultsEnd");
}

void EWrapperQThreaded::wshMetaData(int reqId, const std::string& dataJson)
{
    emit signalWshEventData(reqId, dataJson);
}

void EWrapperQThreaded::wshEventData(int reqId, const std::string& dataJson)
{
    emit signalWshMetaData(reqId, dataJson);
}

void EWrapperQThreaded::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId,
    const std::string& tradingClass, const std::string& multiplier, const std::set<std::string>& expirations,
    const std::set<double>& strikes)
{
    emit signalSecurityDefinitionOptionalParameter(
        reqId, exchange, underlyingConId, tradingClass, multiplier, expirations, strikes);
}

void EWrapperQThreaded::securityDefinitionOptionalParameterEnd(int reqId)
{
    emit signalSecurityDefinitionOptionalParameterEnd(reqId);
}

void EWrapperQThreaded::marketDataType(TickerId reqId, int marketDataType)
{
    emit signalMarketDataType(reqId, marketDataType);
}
