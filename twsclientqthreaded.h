#ifndef TWSCLIENTQTHREADED_H
#define TWSCLIENTQTHREADED_H

#include "EClientSocket.h"
#include "EReader.h"
#include "EReaderOSSignal.h"

#include <QObject>
#include <QPlainTextEdit>
// #include "EWrapper.h"
#include "ewrapperqthreaded.h"
// #include "ewrapperimp.h"
#include "ReqIdTypes.h"

#include <QThread>

class TwsClientQThreaded : public QThread
{
    Q_OBJECT

public:
    TwsClientQThreaded();
    TwsClientQThreaded(QString ip, int port, int clientId);
    TwsClientQThreaded(QString ip, int port, std::string accountId, int clientId);
    // TwsClientQThreaded(QString ip, int port, int clientId, EWrapper* wrapperImp);
    // TwsClientQThreaded(QString ip, int port, int clientId, std::shared_ptr<EWrapper> wrapperImp);
    ~TwsClientQThreaded();

    bool connect();
    void disconnect();
    void processMsg();
    void startMsgProcessing();
    void cancelAllReqs();

    void run() override // qthread virtual overrid
    {
        processMsg();
    }

    void reqNextValidOrderId()
    {
        clientSocket->reqIds(0);
    }

    void reqAccountSummary();
    void reqAccountUpdates(std::string accountNumber);
    //void reqAccountUpdates(std::string accountNumber, int reqId);
    void reqPositionUpdates();
    void reqPositionUpdates(int reqId);
    void cancelPositionUpdates();
    void cancelPositionUpdates(int reqId);
    void cancelAccountSummary();
    void cancelAccountUpdates();

    void reqContractDetails(Contract contract);
    void reqContractDetails(int reqId, Contract contract);

    void reqHistoricalData(Contract contract, std::string endDateTime, std::string durationStr,
        std::string barSizeSetting, std::string whatToShow, int useRTH, int formatDate, bool keepUpToDate);
    void cancelHistoricalData();
    void reqHistoricalDataId(int reqId, Contract contract, std::string endDateTime, std::string durationStr,
        std::string barSizeSetting, std::string whatToShow, int useRTH, int formatDate, bool keepUpToDate);
    void cancelHistoricalDataId(int reqId);

    void reqScannerSubscription(const ScannerSubscription& subscription,
        const TagValueListSPtr& scannerSubscriptionOptions,
        const TagValueListSPtr& scannerSubscriptionFilterOptions);
    void cancelScannerSubscription();

    void reqMarketDataType(int typeId);
    // 5 second bars
    // whatToShow can be "TRADES, MIDPOINT, BID, ASK"
    void reqRealTimeBars(Contract contract, std::string whatToShow, bool useRTH);
    void cancelRealTimeBars();

    void reqMktData(int reqId, Contract contract, std::string ticklist, bool snapshot, bool regulatorySnapshot, TagValueListSPtr options);
    void reqTickByTickDataBidAsk(Contract contract);
    void reqTickByTickDataMid(int reqid, Contract contract, int ticks);
    void cancelTickByTickData();
    void cancelTickByTickData(int reqid);
    void cancelReqMktData(int reqId);

    void placeOrder(Contract con, Order order);
    void placeOrder(OrderId id, Contract con, Order order);

    void reqWshEventData(WshEventData eventData);
    void reqWshMetaEventData();

    void reqSecDefOptParams(int id, std::string underlyingSymbol, std::string futFopExchange, std::string underlyingSecType, int underlyingConId);

    /*reportType = ReportSnapshot, ReportsFinSummary, ReportRatios, ReportsFinStatements, RESC*/
    void reqFundamentals(
        const Contract& con, const std::string& reportType, const TagValueListSPtr& fundamentalDataOptions);

    QString ip;
    int port;
    int clientId;
    EReaderOSSignal* signal;
    EClientSocket* clientSocket;
    EReader* reader;
    EWrapperQThreaded* qewrapper;
    // EWrapperImp* ewrapper;
    std::thread msgProcessThread;
    QThread* qMsgProcessThread;

    // std::map<int, RequestType> reqIdMap;
    std::vector<unsigned> activeReqs;
    unsigned reqIdCounter{0};
    bool connected{false};
    std::string accountNumber;

    OrderId currOrderId;
    std::string accountId;

signals:
};

#endif // TwsClientQThreadedQTHREADED_H
