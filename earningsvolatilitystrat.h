#ifndef EARNINGSVOLATILITYSTRAT_H
#define EARNINGSVOLATILITYSTRAT_H

#include "strategies.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

#include "rapidJsonHelpers.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"

#include <iomanip>
#include <sstream>

class EarningsVolatilityStrat : public BaseAgent
{
    enum OptionRight
    {
        PUT,
        CALL
    };

    struct UnixBar : public Bar
    {
        time_t unixts;
    };

    struct HistoryIVStruct
    {
        int reqId;
        std::string symbol;
        std::vector<UnixBar> ivCandles;
        double currentIVPercentile;
    };

    struct FinnHubEarnings
    {
        std::string date;
        double epsActual;
        double epsEstimate;
        std::string hour;
        uint8_t quarter;
        int64_t revActual;
        int64_t revEstimate;
        std::string symbol;
        uint16_t year;
    };

    struct Option
    {
        int reqPriceId;
        int bidSize;
        int askSize;
        double bidPrice{-99};
        double askPrice{-99};
        double midPrice{-99};
        double strike;
        std::string expiration;
    };

    struct OptionsChain
    {
        std::set<std::string> expirations;
        std::set<double> strikes;
    };

    struct ContractDetails_Ext
    {
        int reqId;
        FinnHubEarnings earnings;
        ContractDetails details;
        double currentMidPrice;
        OptionsChain optionsChain;

        bool operator==(const ContractDetails_Ext& other)
        {
            return this->details.contract.symbol == other.details.contract.symbol;
        }
    };

    struct SuperContract_Ea : public SuperContract
    {
        int reqId;
        FinnHubEarnings earnings;
        //ContractDetails details;
        double currentMidPrice;
        bool operator==(const SuperContract_Ea& other)
        {
            return this->contract.symbol == other.contract.symbol;
        }
    };

    struct StrangleOrder
    {
        Option callSide;
        Option putSide;
        Contract contract;
    };

    Q_OBJECT
public:
    EarningsVolatilityStrat(TwsClientQThreaded* client);
    EarningsVolatilityStrat();
    ~EarningsVolatilityStrat()
    {
        //delete netAccessManager;
        client->disconnect();
    }

    void runStrat() override;

private:
    void parseEarningsCalendarFromJsonFile();
    void parseEarningsCalendarJson(rapidjson::Document& doc);
    void scanMarket();
    double calculatePercentile(std::vector<UnixBar> ivs)
    {
        auto currIV = ivs[ivs.size() - 1];
        int countsBelowCurrentIV = 0;
        for (unsigned i = 0; i < ivs.size() - 2; i++)
        {
            if (ivs[i].close < currIV.close)
            {
                countsBelowCurrentIV++;
            }
        }
        return static_cast<double>(countsBelowCurrentIV) / static_cast<double>((ivs.size() - 1)) * 100.0;
    }

    void getLastPrice(std::vector<SuperContract_Ea>& contracts);
    void selectOptionsForStrangle(SuperContract_Ea contract);
    void filterOptions();
    void filterOptionsChain(ContractDetails_Ext contract);
    void sendStrangleOrdersIfReady(StrangleOrder strangle);

    std::time_t stringTimeToUnix(std::string str, std::string format)
    {
        std::tm t{};
        std::istringstream ss(str);

        ss >> std::get_time(&t, format.c_str());
        if (ss.fail()) {
            throw std::runtime_error{"failed to parse time string"};
        }

        return mktime(&t);
    }

    std::string unixTimeToString(std::time_t& t, std::string format)
    {
        std::tm tmtime = *std::localtime(&t);
        std::stringstream buff;
        buff << std::put_time(&tmtime, format.c_str());
        return buff.str();
    }

    std::vector<ContractDetails> scanResults;
    std::vector<SuperContract_Ea> contractDetailsWithEarnings;
    std::vector<HistoryIVStruct> historicalIVData;
    std::vector<FinnHubEarnings> earningsCalendar;
    std::vector<SuperContract_Ea> selectedContractsToTrade;
    std::vector<StrangleOrder> strangles;

    unsigned expecedHistoricalDataEnds{0};
    unsigned historicalDataEndsCounter{0};
    unsigned selectedContractsLastPriceCounter{0};
    unsigned selectedContractsToTradeOptionsReqCounter{0};
    unsigned selectedContractOptionsFilteredCounter{0};
    unsigned strangleOrdersCounter{0};
    unsigned strangleCallOrdersCounter{0};
    unsigned stranglePutOrdersCounter{0};
    int optionsPriceReqId{4000};

    QNetworkAccessManager* netAccessManager;

signals:
    void signalRequestOptionsChain(std::vector<Contract> contracts);

private slots:
    void onManagerFinished(QNetworkReply* reply);

public slots:
    //signal from OptionsChainAgent
    void onRecieveOptionsChain(std::vector<SuperContract> scontracts);

    //ewrapper signals
    void onSignalError(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson) override;
    void onSignalLogger(QString msg) override;
    void onSignalHistoricalDataBarEndData(int reqId, std::vector<Bar> bars) override;
    void onSignalScanResultsDone(std::vector<ContractDetails> results) override;
    void onSignalTickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize,
        Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) override;
    void onSignalTickByTickMid(int reqId, time_t time, double midPoint) override;
    void onSignalTickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib) override;
    void onSignalMarketDataType(TickerId tickerId, int type) override;
};

#endif // EARNINGSVOLATILITYSTRAT_H
