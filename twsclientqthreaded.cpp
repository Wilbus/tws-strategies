#include "twsclientqthreaded.h"

#include <QDebug>
#include <thread>

TwsClientQThreaded::TwsClientQThreaded()
{
}

TwsClientQThreaded::TwsClientQThreaded(QString ip, int port, int clientId)
    : ip(ip)
    , port(port)
    , clientId(clientId)
{
    signal = new EReaderOSSignal();
    qewrapper = new EWrapperQThreaded(clientId);
    clientSocket = new EClientSocket(qewrapper, signal);
}

TwsClientQThreaded::TwsClientQThreaded(QString ip, int port, std::string accId, int clientId)
    : ip(ip)
    , port(port)
    , clientId(clientId)
{
    signal = new EReaderOSSignal();
    qewrapper = new EWrapperQThreaded(clientId);
    clientSocket = new EClientSocket(qewrapper, signal);
    accountNumber = accId;
}

bool TwsClientQThreaded::connect()
{
    bool conn = clientSocket->eConnect(ip.toStdString().c_str(), port, clientId, false);
    if (conn)
    {
        reader = new EReader(clientSocket, signal);
        reader->start(); // reader thread start
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);

        //reqAccountUpdates("DU8491376");
        //reqPositionUpdates();
        //reqNextValidOrderId();
        connected = true;
    }
    return conn;
}

void TwsClientQThreaded::disconnect()
{
    if(connected)
        clientSocket->eDisconnect();
}

void TwsClientQThreaded::processMsg() // message process thread
{
    // reads from readerqueue when signaled
    // should call wrapperimp functions automatically
    while (true)
    {
        // qDebug("qt process events");
        // qDebug("processMsgWait\n");
        signal->waitForSignal();
        reader->processMsgs();
        // qDebug("processMsgDone\n");
    }
}

void TwsClientQThreaded::startMsgProcessing()
{
    msgProcessThread = std::thread(&TwsClientQThreaded::processMsg, this);
    // qMsgProcessThread = new QThread();
}

void TwsClientQThreaded::cancelAllReqs()
{
    //for (const auto& id : activeReqs)
    //{
    //}
    //cancelAccountUpdates();
    //cancelRealTimeBars();
}

void TwsClientQThreaded::reqAccountSummary()
{
    clientSocket->reqAccountSummary(ReqIdType::reqAccountSummary, "All", "$LEDGER:USD");
}

void TwsClientQThreaded::reqAccountUpdates(std::string accountNumber)
{
    this->accountNumber = accountNumber;
    clientSocket->reqAccountUpdates(true, accountNumber);
}

void TwsClientQThreaded::reqPositionUpdates()
{
    // clientSocket->reqPositions();
    clientSocket->reqPositionsMulti(ReqIdType::reqPostionUpdates, accountNumber, "");
}

void TwsClientQThreaded::reqPositionUpdates(int reqId)
{
    // clientSocket->reqPositions();
    clientSocket->reqPositionsMulti(reqId, accountNumber, "");
}

void TwsClientQThreaded::cancelPositionUpdates()
{
    // clientSocket->cancelPositions();
    clientSocket->cancelPositionsMulti(ReqIdType::reqPostionUpdates);
}

void TwsClientQThreaded::cancelPositionUpdates(int reqId)
{
    // clientSocket->cancelPositions();
    clientSocket->cancelPositionsMulti(reqId);
}

void TwsClientQThreaded::cancelAccountSummary()
{
    clientSocket->cancelAccountSummary(ReqIdType::reqAccountSummary);
}

void TwsClientQThreaded::cancelAccountUpdates()
{
    clientSocket->reqAccountUpdates(false, accountNumber);
}

void TwsClientQThreaded::reqScannerSubscription(const ScannerSubscription& subscription,
    const TagValueListSPtr& scannerSubscriptionOptions,
    const TagValueListSPtr& scannerSubscriptionFilterOptions)
{
    qewrapper->contractScanResultsBuff.clear();
    clientSocket->reqScannerSubscription(
        ReqIdType::reqScannerSubscription, subscription, scannerSubscriptionOptions, scannerSubscriptionFilterOptions);
}

void TwsClientQThreaded::cancelScannerSubscription()
{
    clientSocket->cancelScannerSubscription(ReqIdType::reqScannerSubscription);
}

void TwsClientQThreaded::reqContractDetails(Contract contract)
{
    qDebug() << contract.symbol << ", " << contract.secType << ", " << contract.exchange << ", " << contract.currency
             << " \n";
    clientSocket->reqContractDetails(ReqIdType::reqContractDetails, contract);
}

void TwsClientQThreaded::reqContractDetails(int reqId, Contract contract)
{
    clientSocket->reqContractDetails(reqId, contract);
}

void TwsClientQThreaded::reqHistoricalData(Contract contract, std::string endDateTime, std::string durationStr,
    std::string barSizeSetting, std::string whatToShow, int useRTH, int formatDate, bool keepUpToDate)
{
    qewrapper->historicalDataBuff.clear();
    clientSocket->reqHistoricalData(ReqIdType::reqHistoricalData, contract, endDateTime, durationStr, barSizeSetting,
        whatToShow, useRTH, formatDate, keepUpToDate, TagValueListSPtr());
}

void TwsClientQThreaded::cancelHistoricalData()
{
    clientSocket->cancelHistoricalData(ReqIdType::reqHistoricalData);
}

void TwsClientQThreaded::reqHistoricalDataId(int id, Contract contract, std::string endDateTime,
    std::string durationStr, std::string barSizeSetting, std::string whatToShow, int useRTH, int formatDate,
    bool keepUpToDate)
{
    qewrapper->historicalDataBuff.clear();
    clientSocket->reqHistoricalData(id, contract, endDateTime, durationStr, barSizeSetting, whatToShow, useRTH,
        formatDate, keepUpToDate, TagValueListSPtr());
}

void TwsClientQThreaded::cancelHistoricalDataId(int id)
{
    clientSocket->cancelHistoricalData(id);
}

void TwsClientQThreaded::reqRealTimeBars(Contract contract, std::string whatToShow, bool useRTH)
{
    clientSocket->reqRealTimeBars(ReqIdType::reqRealTimeBars, contract, 5, whatToShow, useRTH, TagValueListSPtr());
}

void TwsClientQThreaded::cancelRealTimeBars()
{
    clientSocket->cancelRealTimeBars(ReqIdType::reqRealTimeBars);
}

void TwsClientQThreaded::reqMarketDataType(int type)
{
    clientSocket->reqMarketDataType(type);
}

void TwsClientQThreaded::reqTickByTickDataBidAsk(Contract contract)
{
    clientSocket->reqTickByTickData(ReqIdType::reqTickByTickBidAsk, contract, "BidAsk", tickByTickCount, false);
}

void TwsClientQThreaded::cancelTickByTickData()
{
    clientSocket->cancelTickByTickData(ReqIdType::reqTickByTickBidAsk);
}

void TwsClientQThreaded::cancelTickByTickData(int reqid)
{
    clientSocket->cancelTickByTickData(reqid);
}

void TwsClientQThreaded::reqTickByTickDataMid(int reqid, Contract contract, int ticks)
{
    clientSocket->reqTickByTickData(reqid, contract, "MidPoint", ticks, true);
}

void TwsClientQThreaded::reqMktData(int reqId, Contract contract, std::string ticklist, bool snapshot, bool regulatorySnapshot, TagValueListSPtr options)
{
    clientSocket->reqMktData(reqId, contract, ticklist, snapshot, regulatorySnapshot, options);
}

void TwsClientQThreaded::cancelReqMktData(int reqId)
{
    clientSocket->cancelMktData(reqId);
}

void TwsClientQThreaded::placeOrder(Contract con, Order order)
{
    clientSocket->placeOrder(currOrderId, con, order);
    reqNextValidOrderId();
}

void TwsClientQThreaded::placeOrder(OrderId id, Contract con, Order order)
{
    clientSocket->placeOrder(id, con, order);
}

void TwsClientQThreaded::reqWshEventData(WshEventData eventData)
{
    clientSocket->reqWshEventData(ReqIdType::reqWshEventData, eventData);
}

void TwsClientQThreaded::reqWshMetaEventData()
{
    clientSocket->reqWshMetaData(ReqIdType::reqWshEventData);
}

void TwsClientQThreaded::reqFundamentals(
    const Contract& con, const std::string& reportType, const TagValueListSPtr& fundamentalDataOptions)
{
    clientSocket->reqFundamentalData(10, con, reportType, fundamentalDataOptions);
}

void TwsClientQThreaded::reqSecDefOptParams(int id, std::string underlyingSymbol, std::string futFopExchange, std::string underlyingSecType, int underlyingConId)
{
    clientSocket->reqSecDefOptParams(id, underlyingSymbol, futFopExchange, underlyingSecType, underlyingConId);
}

TwsClientQThreaded::~TwsClientQThreaded()
{
    // cancelAllReqs();
    // disconnect();
    delete reader;
    delete clientSocket;
    delete signal;
}
