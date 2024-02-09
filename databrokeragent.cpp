#include "databrokeragent.h"
#include "marketdatasingleton.h"

DataBroker::DataBroker()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", AgentClientIds::DataBrokerClient);
    connectSlots();
    client->connect();
    client->start();
}

void DataBroker::onSignalTickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib)
{
    auto msg = fmtlog(logger, "%s: reqId %d %s %.02f", __func__, tickerId, tickTypeToString.at(field).c_str(), price);
    emit signalPassLogMsg(msg);

    if(reqIdMap.find(tickerId) != reqIdMap.end())
    {
        auto databank = MarketDataSingleton::GetInstance();
        auto contract = reqIdMap[tickerId];
        if(contract.secType == "OPT")
        {
            auto optionName = optionContractString(contract);
            databank->updateMkData(optionName, field, price);
        }
        else if(contract.secType == "STK")
        {
            databank->updateMkData(reqIdMap[tickerId].symbol, field, price);
        }
    }
    else
    {
        //something wrong happened, the reqid in callback doesnt match any
        //existing reqid's we made
    }
}

void DataBroker::onSubscribeMktData(Contract contract)
{
    QString msg;
    if(contract.secType == "STK")
        msg = fmtlog(logger, "%s: subscribe to %s mktData with reqId %d", __func__, contract.symbol.c_str(), reqIdCounter);
    else if(contract.secType == "OPT")
        msg = fmtlog(logger, "%s: subscribe to %s mktData with reqId %d", __func__, optionContractString(contract).c_str(), reqIdCounter);
    emit signalPassLogMsg(msg);
    client->reqMarketDataType(MarketDataTypes::Frozen);
    client->reqMktData(reqIdCounter, contract, "", false, false, TagValueListSPtr());
    reqIdMap[reqIdCounter] = contract;
    reqIdCounter++;
}

void DataBroker::onUnsubscribeMktData(Contract contract)
{
    for(auto it = reqIdMap.begin(); it != reqIdMap.end(); ++it)
    {
        if(it->second.symbol == contract.symbol)
        {
            auto msg = fmtlog(logger, "%s: unsubscribe from %s mktData", __func__, contract.symbol.c_str());
            emit signalPassLogMsg(msg);

            client->cancelReqMktData(it->first);
            reqIdMap.erase(it);
            auto databank = MarketDataSingleton::GetInstance();
            databank->resetMktData(contract.symbol);
            break;
        }
    }
}
