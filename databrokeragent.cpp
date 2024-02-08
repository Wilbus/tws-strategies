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
    auto msg = fmtlog(logger, "%s: reqId %d", __func__, tickerId);
    emit signalPassLogMsg(msg);

    if(reqIdMap.find(tickerId) != reqIdMap.end())
    {
        auto databank = MarketDataSingleton::GetInstance();
        databank->updateMkData(reqIdMap[tickerId].symbol, field, price);
    }
    else
    {
        //something wrong happened, the reqid in callback doesnt match any
        //existing reqid's we made
    }
}

void DataBroker::onSubscribeMktData(Contract contract)
{
    auto msg = fmtlog(logger, "%s: subscribe to %s mktData with reqId %d", __func__, contract.symbol.c_str(), reqIdCounter);
    emit signalPassLogMsg(msg);

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
            break;
        }
    }
}
