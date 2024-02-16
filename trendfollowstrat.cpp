#include "trendfollowstrat.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

TrendFollowStrat::TrendFollowStrat()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", AgentClientIds::TrendFollowAgentClient);
    connectSlots();

    client->connect();
    client->start();

    connect(this, SIGNAL(receivedAllHistoricalData()), this, SLOT(onReceivedAllHistoricalData()));
}

void TrendFollowStrat::runStrat()
{
    watchListHistoricalDataCount = 0;
    reqsCounter = ReqIdStarts::TrendFollowAgentReqs;
    //TODO: get current open positions
    //get list of contracts from watchlist (from json file or tws)
    //calculate indicators from historical
    //watch realtime data

    INFOLOG("%s start", __func__);
    watchList = getWatchList();
    getWatchListHistorical();

}

std::vector<Contract> TrendFollowStrat::getWatchList()
{
    //TODO: make relative path available
    FILE* fp = fopen("/datadisk0/sambashare0/coding/qtdesignstudio/strategiesbotwidget/config/trendwatchlist.json", "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document d;
    d.ParseStream(is);

    if(!d.IsObject())
    {
        emit signalPassLogMsg(fmtlog(logger, "%s: Error parsing watchlist json", __func__));
    }

    std::vector<Contract> contracts;
    if (d.HasMember("watchlist") && d["watchlist"].IsArray())
    {
        for(const auto& symbol : d["watchlist"].GetArray())
        {
            //INFOLOG("%s: %s", __func__, symbol.GetString());
            Contract c;
            c.symbol = symbol.GetString();
            c.secType = "STK";
            c.exchange = "SMART";
            c.currency = "USD";
            contracts.push_back(c);
        }
    }
    return contracts;
}

void TrendFollowStrat::getWatchListHistorical()
{
    std::time_t t = std::time(nullptr);
    std::tm nowtime = *std::localtime(&t);
    std::stringstream buff;
    buff << std::put_time(&nowtime, "%Y%m%d-%H:%M:%S");

    INFOLOG("%s", __func__);

    for(const auto& con : watchList)
    {
        watchListHistoricals[reqsCounter] = WatchListHistorical();
        watchListHistoricals.at(reqsCounter).contract = con;
        //TODO: maybe we can also include option IV and hist volatility
        client->reqHistoricalDataId(
            reqsCounter, con, buff.str().c_str(), "1 Y", "1 day", "TRADES", 0, 1, false);

        reqsCounter += 1;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }
}

void TrendFollowStrat::onSignalHistoricalDataBarEndData(int reqId, std::vector<Bar> bars)
{
    if(watchListHistoricals.find(reqId) == watchListHistoricals.end())
    {
        INFOLOG("%s: WARN: reqid %d is not a key in watchListHistoricals", __func__, reqId);
        return;
    }
    INFOLOG("%s reqId %d historicalData received", __func__, reqId);
    watchListHistoricals.at(reqId).bars = bars;
    watchListHistoricalDataCount += 1;
    if(watchListHistoricalDataCount == watchListHistoricals.size())
    {
        emit receivedAllHistoricalData();
    }
}

void TrendFollowStrat::onReceivedAllHistoricalData()
{
    INFOLOG("%s", __func__);
}
