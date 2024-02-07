#include "earningsvolatilitystrat.h"

#include "ScannerSubscription.h"

#include <algorithm>
#include <iomanip>
#include <thread>
#include <cstdio>




using namespace std;

EarningsVolatilityStrat::EarningsVolatilityStrat(TwsClientQThreaded* client)
{
    this->client = client;
}

EarningsVolatilityStrat::EarningsVolatilityStrat()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", AgentClientIds::EarningsVolatilityStratClient);
    connectSlots();

    client->connect();
    client->start();
}

void EarningsVolatilityStrat::runStrat()
{
    scanResults.clear();
    contractDetailsWithEarnings.clear();
    historicalIVData.clear();
    earningsCalendar.clear();
    selectedContractsToTrade.clear();
    strangles.clear();
    expecedHistoricalDataEnds = 0;
    historicalDataEndsCounter = 0;
    selectedContractsLastPriceCounter = 0;
    selectedContractsToTradeOptionsReqCounter = 0;
    selectedContractOptionsFilteredCounter = 0;
    strangleOrdersCounter = 0;
    strangleCallOrdersCounter = 0;
    stranglePutOrdersCounter = 0;
    optionsPriceReqId = 4000;
    /* get earnings calendar from finnhub
     * scan contracts of certain criteria from tws
     * look in earnings calendar using scan results and extract earnings date
     * get hist iv
     * calcualte current hist IV percentile
     *
     */
    netAccessManager = new QNetworkAccessManager();
    connect(netAccessManager, &QNetworkAccessManager::finished, this, &EarningsVolatilityStrat::onManagerFinished);

    std::time_t t = std::time(nullptr);
    t += 432000; //5 days later than now
    std::time_t t2 = t + 1.21e+6; // 2 weeks range
    std::tm nowtime = *std::localtime(&t);
    std::tm endtime = *std::localtime(&t2);

    std::stringstream buff;
    std::stringstream buff2;
    buff << std::put_time(&nowtime, "%Y-%m-%d");
    buff2 << std::put_time(&endtime, "%Y-%m-%d");

    auto finnHubToken = QString("cfk48j9r01qvg1mcron0cfk48j9r01qvg1mcrong");
    QNetworkRequest request;
    QString requestStr = QString("https://finnhub.io/api/v1/calendar/earnings?from=%1&to=%2&token=%3")
                             .arg(QString(buff.str().c_str()), QString(buff2.str().c_str()), finnHubToken);
    emit signalPassLogMsg(QString("GET %1").arg(requestStr));
    request.setUrl(requestStr);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netAccessManager->get(request);
}

void EarningsVolatilityStrat::parseEarningsCalendarFromJsonFile()
{
    emit signalPassLogMsg("Parsing earnings calendar from cached json file as backup action");
    FILE* fp = fopen("/datadisk0/sambashare0/coding/qtdesignstudio/earningsCalendar_20230203_20230217.json", "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document d;
    d.ParseStream(is);
    parseEarningsCalendarJson(d);
}

void EarningsVolatilityStrat::parseEarningsCalendarJson(rapidjson::Document& doc)
{
    if (!doc.IsObject())
    {
        emit signalPassLogMsg("Error parsing finnhub earnings calendar json from file\n");
    }
    if (doc.HasMember("earningsCalendar") && doc["earningsCalendar"].IsArray())
    {
        auto earningsArray = doc["earningsCalendar"].GetArray();
        int counter = 0;
        for (const auto& d : earningsArray)
        {
            FinnHubEarnings e;
            PARSE_STRING(e.date, "date");
            // e.date = earnings["date"].GetString();
            PARSE_DOUBLE(e.epsActual, "epsActual");
            PARSE_DOUBLE(e.epsEstimate, "epsEstimate");
            PARSE_STRING(e.hour, "hour");
            PARSE_INT(e.quarter, "quarter");
            PARSE_INT(e.revActual, "revenueActual");
            PARSE_INT(e.revEstimate, "revenueEstimate");
            PARSE_STRING(e.symbol, "symbol");
            // e.symbol = earnings["symbol"].GetString();
            PARSE_INT(e.year, "year");

            earningsCalendar.push_back(e);
            counter++;
        }
    }
}

void EarningsVolatilityStrat::onManagerFinished(QNetworkReply* reply)
{
    if (reply->error())
    {
        emit signalPassLogMsg("ERROR\n");
        emit signalPassLogMsg(reply->errorString() + "\n");
        parseEarningsCalendarFromJsonFile();
        scanMarket();
        return;
    }

    QString jsonString = reply->readAll();

    // emit signalPassLogMsg(jsonString + "\n");

    if (jsonString.length() == 0)
    {
        int test = 0;
    }

    rapidjson::Document doc;
    doc.Parse(jsonString.toStdString().c_str());
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    doc.Accept(writer);

    if (!doc.IsObject())
    {
        emit signalPassLogMsg("Error parsing finnhub earnings calendar json from online, falling back to json files\n");
        parseEarningsCalendarFromJsonFile();
        scanMarket();
        return;
    }
    if (doc.HasMember("earningsCalendar") && doc["earningsCalendar"].IsArray())
    {
        auto earningsArray = doc["earningsCalendar"].GetArray();
        int counter = 0;
        for (const auto& d : earningsArray)
        {
            FinnHubEarnings e;
            PARSE_STRING(e.date, "date");
            // e.date = earnings["date"].GetString();
            PARSE_DOUBLE(e.epsActual, "epsActual");
            PARSE_DOUBLE(e.epsEstimate, "epsEstimate");
            PARSE_STRING(e.hour, "hour");
            PARSE_INT(e.quarter, "quarter");
            PARSE_INT(e.revActual, "revenueActual");
            PARSE_INT(e.revEstimate, "revenueEstimate");
            PARSE_STRING(e.symbol, "symbol");
            // e.symbol = earnings["symbol"].GetString();
            PARSE_INT(e.year, "year");

            earningsCalendar.push_back(e);
            counter++;
        }
    }

    emit signalPassLogMsg(QString("earningsCalendar has %1 results\n").arg(earningsCalendar.size()));
    scanMarket();
}

void EarningsVolatilityStrat::scanMarket()
{
    client->cancelScannerSubscription();

    ScannerSubscription ss;
    ss.instrument = "STK";
    ss.locationCode = "STK.US.MAJOR";
    ss.scanCode = "HOT_BY_OPT_VOLUME";
    ss.abovePrice = 5;
    ss.belowPrice = 500;
    ss.aboveVolume = 500000;
    //ss.marketCapAbove = 1000000;
    // ss.marketCapAbove = scannerControls->marketCapAboveCombo->currentText().toDouble() * 1e6;
    // ss.marketCapBelow = scannerControls->marketCapBelowCombo->currentText().toDouble() * 1e6;
    //ss.averageOptionVolumeAbove = 1000;

    TagValueListSPtr tagList(new TagValueList());
    client->reqScannerSubscription(ss, TagValueListSPtr(), tagList);
}

void EarningsVolatilityStrat::onSignalScanResultsDone(std::vector<ContractDetails> results)
{
    scanResults = results;
    client->cancelScannerSubscription();

    for (const auto& res : scanResults)
    {
        for (const auto& e : earningsCalendar)
        {
            if (res.contract.symbol == e.symbol)
            {
                SuperContract_Ea ce;
                ce.earnings = e;
                ce.contract = res.contract;
                contractDetailsWithEarnings.push_back(ce);
                break;
            }
        }
    }

    std::time_t t = std::time(nullptr);
    std::tm nowtime = *std::localtime(&t);
    std::stringstream buff;
    buff << std::put_time(&nowtime, "%Y%m%d-%H:%M:%S");

    unsigned reqid = 1000;
    expecedHistoricalDataEnds = contractDetailsWithEarnings.size();
    emit signalPassLogMsg(QString("requesting historicalIV for %1 symbols").arg(expecedHistoricalDataEnds));
    for (const auto& ce : contractDetailsWithEarnings)
    {
        HistoryIVStruct ivstruct;
        ivstruct.reqId = reqid;
        ivstruct.symbol = ce.contract.symbol;
        historicalIVData.push_back(ivstruct);

        emit signalPassLogMsg(QString("getting historical IV for %1").arg(ce.contract.symbol.c_str()));
        client->reqHistoricalDataId(
            reqid, ce.contract, buff.str().c_str(), "1 Y", "1 day", "OPTION_IMPLIED_VOLATILITY", 0, 1, false);
        reqid += 1;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }
}

void EarningsVolatilityStrat::onSignalHistoricalDataBarEndData(int reqId, std::vector<Bar> bars)
{
    for (auto& ivstruct : historicalIVData)
    {
        if (reqId == ivstruct.reqId)
        {
            for (const auto& bar : bars)
            {
                std::tm t{};
                std::istringstream ss(bar.time);
                ss >> std::get_time(&t, "%Y%m%d %H:%M:%S");
                std::time_t timestamp = mktime(&t);
                UnixBar ubar;
                ubar.unixts = timestamp;
                ubar.open = bar.open;
                ubar.high = bar.high;
                ubar.low = bar.low;
                ubar.close = bar.close;

                ivstruct.ivCandles.push_back(ubar);
            }

            std::sort(ivstruct.ivCandles.begin(), ivstruct.ivCandles.end(),
                [](const UnixBar& a, const UnixBar& b) { return a.unixts < b.unixts; });

            QString msg = QString("count: %1, reqID: %2, %3 has %4 iv close values")
                              .arg(QString::number(historicalDataEndsCounter), QString::number(reqId),
                                  QString(ivstruct.symbol.c_str()), QString::number(ivstruct.ivCandles.size()));
            emit signalPassLogMsg(msg);
            historicalDataEndsCounter += 1;
            break;
        }
    }
    // have we received all reqIds?
    if (historicalDataEndsCounter == expecedHistoricalDataEnds)
    {
        emit signalPassLogMsg(QString("received all historical data"));
        for (const auto& ivstruct : historicalIVData)
        {
            auto currIV = ivstruct.ivCandles[ivstruct.ivCandles.size() - 1].close;
            auto currIVPercentile = calculatePercentile(ivstruct.ivCandles);

            QString msg = QString("%1 currIV = %2 currIVP = %3")
                              .arg(QString(ivstruct.symbol.c_str()), QString::number(currIV),
                                  (QString::number(currIVPercentile)));
            emit signalPassLogMsg(msg);

            if (currIVPercentile < 70) // can configure this later
            {
                for (const auto& contract : contractDetailsWithEarnings)
                {
                    if (contract.contract.symbol == ivstruct.symbol)
                    {
                        QString msg = QString("selected %1 with currIV %2, earningsDate: %3")
                                          .arg(QString(ivstruct.symbol.c_str()), QString::number(currIVPercentile),
                                              QString(contract.earnings.date.c_str()));
                        emit signalPassLogMsg(msg);
                        selectedContractsToTrade.push_back(contract);
                    }
                }
            }
        }

        QString msg = QString("selectedContractsToTrade size = %1").arg(QString::number(selectedContractsToTrade.size()));
        emit signalPassLogMsg(msg);
        getLastPrice(selectedContractsToTrade);
    }
}

void EarningsVolatilityStrat::getLastPrice(std::vector<SuperContract_Ea>& contracts)
{
    int id = 2000;

    client->reqMarketDataType(MarketDataTypes::Frozen); //test for when market is closed
    for (auto& con : contracts) //set reqId to associate the reqId with particular contract_ext struct
    {
        //TODO: maybe use this when ready
        //client->reqTickByTickDataMid(id, con.details.contract, 1);
        con.reqId = id;

        emit signalPassLogMsg(QString("reqId %1 reqMktData for %2").arg(
            QString::number(con.reqId),
            QString(con.contract.symbol.c_str())));
        client->reqMktData(id, con.contract, "233", false, false, TagValueListSPtr());
        id += 1;
    }
}

void EarningsVolatilityStrat::onSignalTickByTickMid(int reqId, time_t time, double midPoint)
{
    QString msg = QString("onSignalTickByTickMid %1, %2").arg(QString::number(reqId), QString::number(midPoint));
    emit signalPassLogMsg(msg);
    client->cancelTickByTickData(reqId);
}

void EarningsVolatilityStrat::onSignalTickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib)
{
    /*QString msg;
    if(field > TickType::NOT_SET || field < TickType::BID_SIZE)
        msg = QString("onSignalTickPrice reId %1 ticktype %2").arg(QString::number(tickerId), QString::number(field));
    else
        msg = QString("onSignalTickPrice reId %1 ticktype %2").arg(
            QString::number(tickerId), QString(tickTypeToString.at(field).c_str()));
    emit signalPassLogMsg(msg);*/
    for(auto& con : selectedContractsToTrade)
    {
        if(tickerId == con.reqId)
        {
            switch(field)
            {
                case TickType::LAST:
                case TickType::DELAYED_LAST:
                {
                    client->cancelReqMktData(tickerId);
                    emit signalPassLogMsg(QString("Count: %1 reqid: %2, %3 %4 %5").arg(
                        QString::number(selectedContractsLastPriceCounter),
                        QString::number(tickerId), QString(con.contract.symbol.c_str()),
                        QString(tickTypeToString.at(field).c_str()), QString::number(price)));

                    con.currentMidPrice = price;
                    //only increase counter if its the ticktype we want
                    //because there are different ticktypes returned for the same reqID
                    selectedContractsLastPriceCounter += 1;

                    //emit signalPassLogMsg(QString("Retrieving options contracts"));
                    //getOptionsContracts(selectedContractsToTrade);
                    //getOptionsChains(con);
                    break;
                }
                default:
                {
                    //return if its not the ticktype we want
                    //we don't want to call accidently call getOptionsContracts when
                    //we didn't incremenet selectedContractsLastPriceCounter
                    break;
                }
            }
            break;
        }
    }
    if(selectedContractsLastPriceCounter == selectedContractsToTrade.size())
    {
        emit signalPassLogMsg(QString("Retrieving options contracts"));
        //getOptionsContracts(selectedContractsToTrade);
        std::vector<Contract> cons;
        for(const auto& con : selectedContractsToTrade)
        {
            cons.push_back(con.contract);
        }
        emit signalRequestOptionsChain(cons);
        selectedContractsLastPriceCounter = 0; //reset so we don't call it again
    }
    for(auto& strangle : strangles)
    {
        if(tickerId == strangle.callSide.reqPriceId)
        {
            switch(field)
            {
                case TickType::LAST:
                case TickType::DELAYED_LAST:
                {
                    //client->cancelReqMktData(tickerId);
                    strangle.callSide.midPrice = price;
                    //sendStrangleOrdersIfReady(strangle);
                    break;
                }
                case TickType::BID:
                {
                    emit signalPassLogMsg(QString("reqId %1 Call Option %2, %3 %4 %5 %6").arg(
                        QString::number(tickerId),
                        QString(strangle.contract.symbol.c_str()),
                        QString::number(strangle.callSide.strike),
                        QString(strangle.callSide.expiration.c_str()),
                        QString(tickTypeToString.at(field).c_str()),
                        QString::number(price)));
                    strangle.callSide.bidPrice = price;
                    sendStrangleOrdersIfReady(strangle);
                    break;
                }
                case TickType::ASK:
                {
                    emit signalPassLogMsg(QString("reqId %1 Call Option %2, %3 %4 %5 %6").arg(
                        QString::number(tickerId),
                        QString(strangle.contract.symbol.c_str()),
                        QString::number(strangle.callSide.strike),
                        QString(strangle.callSide.expiration.c_str()),
                        QString(tickTypeToString.at(field).c_str()),
                        QString::number(price)));
                    strangle.callSide.askPrice = price;
                    sendStrangleOrdersIfReady(strangle);
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        else if(tickerId == strangle.putSide.reqPriceId)
        {
            switch(field)
            {
                case TickType::LAST:
                case TickType::DELAYED_LAST:
                {
                    //client->cancelReqMktData(tickerId);
                    /*emit signalPassLogMsg(QString("reqId %1 Put Option %2, %3 %4 %5 %6").arg(
                        QString::number(tickerId),
                        QString(strangle.contract.symbol.c_str()),
                        QString::number(strangle.callSide.strike),
                        QString(strangle.callSide.expiration.c_str()),
                        QString(tickTypeToString.at(field).c_str()),
                        QString::number(price)));*/
                    strangle.putSide.midPrice = price;
                    //sendStrangleOrdersIfReady(strangle);
                    break;
                }
                case TickType::BID:
                {
                    emit signalPassLogMsg(QString("reqId %1 Put Option %2, %3 %4 %5 %6").arg(
                        QString::number(tickerId),
                        QString(strangle.contract.symbol.c_str()),
                        QString::number(strangle.callSide.strike),
                        QString(strangle.putSide.expiration.c_str()),
                        QString(tickTypeToString.at(field).c_str()),
                        QString::number(price)));
                    strangle.putSide.bidPrice = price;
                    sendStrangleOrdersIfReady(strangle);
                    break;
                }
                case TickType::ASK:
                {
                    emit signalPassLogMsg(QString("reqId %1 Put  Option %2, %3 %4 %5 %6").arg(
                        QString::number(tickerId),
                        QString(strangle.contract.symbol.c_str()),
                        QString::number(strangle.callSide.strike),
                        QString(strangle.putSide.expiration.c_str()),
                        QString(tickTypeToString.at(field).c_str()),
                        QString::number(price)));
                    strangle.putSide.askPrice = price;
                    sendStrangleOrdersIfReady(strangle);
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
    }
}

void EarningsVolatilityStrat::sendStrangleOrdersIfReady(StrangleOrder strangle)
{
    if(strangle.callSide.bidPrice > 0 && strangle.callSide.askPrice > 0 &&
        strangle.putSide.bidPrice > 0 && strangle.putSide.askPrice > 0)
    {
        client->cancelReqMktData(strangle.callSide.reqPriceId);
        client->cancelReqMktData(strangle.putSide.reqPriceId);

        Contract contractC;
        contractC.symbol = strangle.contract.symbol;
        contractC.secType = "OPT";
        contractC.right = "C";
        contractC.strike = strangle.callSide.strike;
        contractC.exchange = strangle.contract.exchange;
        contractC.lastTradeDateOrContractMonth = strangle.callSide.expiration;

        Order callOrder;
        callOrder.action = "BUY";
        callOrder.totalQuantity = DecimalFunctions::stringToDecimal("1");
        callOrder.orderType = "MKT";
        Decimal cbidPriceD = DecimalFunctions::stringToDecimal(std::to_string(strangle.callSide.bidPrice));
        Decimal caskPriceD = DecimalFunctions::stringToDecimal(std::to_string(strangle.callSide.askPrice));
        Decimal cmidPriceD = (cbidPriceD + caskPriceD) / 2;
        double cmidPrice = DecimalFunctions::decimalToDouble(cmidPriceD);
        callOrder.lmtPrice = std::ceil(cmidPrice * 100.0) / 100.0;
        callOrder.transmit = false;
        callOrder.algoStrategy = "Adaptive";
        callOrder.algoParams.reset(new TagValueList());
        TagValueSPtr tagc(new TagValue("adaptivePriority", "Urgent"));
        callOrder.algoParams->push_back(tagc);

        Contract contractP;
        contractP.symbol = strangle.contract.symbol;
        contractP.secType = "OPT";
        contractP.right = "P";
        contractP.strike = strangle.putSide.strike;
        contractP.exchange = strangle.contract.exchange;
        contractP.lastTradeDateOrContractMonth = strangle.putSide.expiration;

        Order putOrder;
        putOrder.action = "BUY";
        putOrder.totalQuantity = DecimalFunctions::stringToDecimal("1");
        putOrder.orderType = "MKT";
        Decimal pbidPriceD = DecimalFunctions::stringToDecimal(std::to_string(strangle.putSide.bidPrice));
        Decimal paskPriceD = DecimalFunctions::stringToDecimal(std::to_string(strangle.putSide.askPrice));
        Decimal pmidPriceD = (pbidPriceD + paskPriceD) / 2;
        double pmidPrice = DecimalFunctions::decimalToDouble(pmidPriceD);
        putOrder.lmtPrice = std::ceil(pmidPrice * 100.0) / 100.0;
        putOrder.transmit = false;
        putOrder.algoStrategy = "Adaptive";
        putOrder.algoParams.reset(new TagValueList());
        TagValueSPtr tagp(new TagValue("adaptivePriority", "Urgent"));
        putOrder.algoParams->push_back(tagp);

        emit signalSendOrderToAccountManager(contractC, callOrder);
        emit signalSendOrderToAccountManager(contractP, putOrder);
    }
}

void EarningsVolatilityStrat::onRecieveOptionsChain(std::vector<SuperContract> scontracts)
{
    for(const auto& scon : scontracts)
    {
        for(auto& selected : selectedContractsToTrade)
        {
            if(scon.contract.symbol == selected.contract.symbol)
            {
                selected.options = scon.options;
            }
        }
    }
    for(auto& selected : selectedContractsToTrade)
    {
        selectOptionsForStrangle(selected);
    }
}

void EarningsVolatilityStrat::selectOptionsForStrangle(SuperContract_Ea contract)
{
    auto earningsDate = contract.earnings.date;
    time_t closestAfterExpDate = std::numeric_limits<time_t>::max();
    std::string closestAfterExpDateStr;
    //we can just choose to look at calls for expiration since calls/puts should have the same expirations
    for(const auto& [exp , callsVec] : contract.options.calls)
    {
        auto unixEarningsDate = stringTimeToUnix(earningsDate, "%Y-%m-%d");
        auto unixExpDate = stringTimeToUnix(exp, "%Y%m%d");

        if(unixExpDate > unixEarningsDate) //is exp date after earnings
        {
            time_t diff = unixExpDate - unixEarningsDate;
            //is exp date at least a week after earnings
            if(diff > 604800 && diff < (closestAfterExpDate - unixExpDate))
            {
                closestAfterExpDate = unixExpDate;
                closestAfterExpDateStr = exp;
            }
        }
    }
    double closestCallStrike = std::numeric_limits<double>::max();
    double closestPutStrike = 0; //0 is the lowest a stock price could go
    //we can just choose to look at calls strikes since calls/puts SHOULD have the same strikes
    for(const auto& callOption : contract.options.calls[closestAfterExpDateStr])
    {
        double diff = callOption.strike - contract.currentMidPrice;
        /*QString dmsg = QString("%1 %2 diff %3").arg(
            QString(contract.details.contract.symbol.c_str()),
            QString::number(strike),
            QString::number(diff));
        emit signalPassLogMsg(dmsg);*/
        if(diff >= 0 && diff < (closestCallStrike - contract.currentMidPrice)) //call
        {
            closestCallStrike = callOption.strike;
        }
        if(diff < 0 && diff > (closestPutStrike - contract.currentMidPrice)) //put
        {
            closestPutStrike = callOption.strike;
        }
    }
    Option callside;
    callside.strike = closestCallStrike;
    callside.expiration = unixTimeToString(closestAfterExpDate, "%Y%m%d");
    Option putside;
    putside.strike = closestPutStrike;
    putside.expiration = unixTimeToString(closestAfterExpDate, "%Y%m%d");
    StrangleOrder strangleOrder{callside, putside, contract.contract};

    QString msg = QString("StrangleOrder->%1 CurrPrice %2 Earnings %3 C %4 %5 P %6 %7").arg(
        QString(contract.contract.symbol.c_str()),
        QString::number(contract.currentMidPrice),
        QString(contract.earnings.date.c_str()),
        QString::number(callside.strike), QString(callside.expiration.c_str()),
        QString::number(putside.strike), QString(putside.expiration.c_str()));
    emit signalPassLogMsg(msg);

    strangles.push_back(strangleOrder);
    strangleOrdersCounter += 1;
    if(strangles.size() == selectedContractsToTrade.size())
    {
        //get current mkt price of option first
        for(auto& strangle : strangles)
        {
            Contract call;
            call.symbol = strangle.contract.symbol;
            call.secType = "OPT";
            call.right = "C";
            call.strike = strangle.callSide.strike;
            call.exchange = strangle.contract.exchange;
            call.lastTradeDateOrContractMonth = strangle.callSide.expiration;
            strangle.callSide.reqPriceId = optionsPriceReqId;

            QString msg = QString("ReqId: %1 ReqMktData for %2 %3 %4 %5").arg(
                QString::number(strangle.callSide.reqPriceId),
                QString(call.symbol.c_str()), QString(call.right.c_str()),
                QString::number(call.strike), QString(call.lastTradeDateOrContractMonth.c_str()));
            emit signalPassLogMsg(msg);
            client->reqMktData(strangle.callSide.reqPriceId, call, "", false, false, TagValueListSPtr());

            optionsPriceReqId += 1;

            Contract put;
            put.symbol = strangle.contract.symbol;
            put.secType = "OPT";
            put.right = "P";
            put.strike = strangle.putSide.strike;
            put.exchange = strangle.contract.exchange;
            put.lastTradeDateOrContractMonth = strangle.putSide.expiration;
            strangle.putSide.reqPriceId = optionsPriceReqId;

            msg = QString("ReqId: %1 ReqMktData for %2 %3 %4 %5").arg(
                QString::number(strangle.putSide.reqPriceId),
                QString(put.symbol.c_str()), QString(put.right.c_str()),
                QString::number(put.strike), QString(put.lastTradeDateOrContractMonth.c_str()));
            emit signalPassLogMsg(msg);
            client->reqMktData(strangle.putSide.reqPriceId, put, "", false, false, TagValueListSPtr());

            optionsPriceReqId += 1;
        }
    }
}

void EarningsVolatilityStrat::filterOptionsChain(ContractDetails_Ext contract)
{
    auto earningsDate = contract.earnings.date;
    time_t closestAfterExpDate = std::numeric_limits<time_t>::max();
    for(const auto& exp : contract.optionsChain.expirations)
    {
        auto unixEarningsDate = stringTimeToUnix(earningsDate, "%Y-%m-%d");
        auto unixExpDate = stringTimeToUnix(exp, "%Y%m%d");

        if(unixExpDate > unixEarningsDate) //is exp date after earnings
        {
            time_t diff = unixExpDate - unixEarningsDate;
            //is exp date at least a week after earnings
            if(diff > 604800 && diff < (closestAfterExpDate - unixExpDate))
            {
                closestAfterExpDate = unixExpDate;
            }
        }
    }
    double closestCallStrike = std::numeric_limits<double>::max();
    double closestPutStrike = 0; //0 is the lowest a stock price could go
    for(const auto& strike : contract.optionsChain.strikes)
    {
        double diff = strike - contract.currentMidPrice;
        /*QString dmsg = QString("%1 %2 diff %3").arg(
            QString(contract.details.contract.symbol.c_str()),
            QString::number(strike),
            QString::number(diff));
        emit signalPassLogMsg(dmsg);*/
        if(diff >= 0 && diff < (closestCallStrike - contract.currentMidPrice)) //call
        {
            closestCallStrike = strike;
        }
        if(diff < 0 && diff > (closestPutStrike - contract.currentMidPrice)) //put
        {
            closestPutStrike = strike;
        }
    }
    Option callside;
    callside.strike = closestCallStrike;
    callside.expiration = unixTimeToString(closestAfterExpDate, "%Y%m%d");
    Option putside;
    putside.strike = closestPutStrike;
    putside.expiration = unixTimeToString(closestAfterExpDate, "%Y%m%d");
    StrangleOrder strangleOrder{callside, putside, contract.details.contract};

    QString msg = QString("StrangleOrder->%1 CurrPrice %2 Earnings %3 C %4 %5 P %6 %7").arg(
        QString(contract.details.contract.symbol.c_str()),
        QString::number(contract.currentMidPrice),
        QString(contract.earnings.date.c_str()),
        QString::number(callside.strike), QString(callside.expiration.c_str()),
        QString::number(putside.strike), QString(putside.expiration.c_str()));
    emit signalPassLogMsg(msg);

    strangles.push_back(strangleOrder);
    strangleOrdersCounter += 1;
    if(strangles.size() == selectedContractsToTrade.size())
    {
        //get current mkt price of option first
        for(auto& strangle : strangles)
        {
            Contract call;
            call.symbol = strangle.contract.symbol;
            call.secType = "OPT";
            call.right = "C";
            call.strike = strangle.callSide.strike;
            call.exchange = strangle.contract.exchange;
            call.lastTradeDateOrContractMonth = strangle.callSide.expiration;
            strangle.callSide.reqPriceId = optionsPriceReqId;

            QString msg = QString("ReqId: %1 ReqMktData for %2 %3 %4 %5").arg(
                QString::number(strangle.callSide.reqPriceId),
                QString(call.symbol.c_str()), QString(call.right.c_str()),
                QString::number(call.strike), QString(call.lastTradeDateOrContractMonth.c_str()));
            emit signalPassLogMsg(msg);
            client->reqMktData(strangle.callSide.reqPriceId, call, "", false, false, TagValueListSPtr());

            optionsPriceReqId += 1;

            Contract put;
            put.symbol = strangle.contract.symbol;
            put.secType = "OPT";
            put.right = "P";
            put.strike = strangle.putSide.strike;
            put.exchange = strangle.contract.exchange;
            put.lastTradeDateOrContractMonth = strangle.putSide.expiration;
            strangle.putSide.reqPriceId = optionsPriceReqId;

            msg = QString("ReqId: %1 ReqMktData for %2 %3 %4 %5").arg(
                QString::number(strangle.putSide.reqPriceId),
                QString(put.symbol.c_str()), QString(put.right.c_str()),
                QString::number(put.strike), QString(put.lastTradeDateOrContractMonth.c_str()));
            emit signalPassLogMsg(msg);
            client->reqMktData(strangle.putSide.reqPriceId, put, "", false, false, TagValueListSPtr());

            optionsPriceReqId += 1;
        }
    }
}

void EarningsVolatilityStrat::filterOptions()
{
    //emit signalPassLogMsg(QString("finding closest later expiration date to earnings date and strikes to current price"));
}

void EarningsVolatilityStrat::onSignalLogger(QString msg)
{
    emit signalPassLogMsg(msg);
}

void EarningsVolatilityStrat::onSignalTickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice,
    Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk)
{
}

void EarningsVolatilityStrat::onSignalMarketDataType(TickerId tickerId, int type)
{
    emit signalPassLogMsg(QString("succesfully set marketdatatype for reqId %1 to %2").arg(
        QString::number(tickerId), QString::number(type)));
}

void EarningsVolatilityStrat::onSignalError(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson)
{
    if(code == 200) //no security definition has been found. strike/exp not valid
    {

    }
}