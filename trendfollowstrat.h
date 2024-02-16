#ifndef TRENDFOLLOWSTRAT_H
#define TRENDFOLLOWSTRAT_H

#include "strategies.h"
#include "MultiCandle.h"

class TrendFollowStrat : public BaseAgent
{
    Q_OBJECT

    struct WatchListHistorical
    {
        Contract contract;
        std::vector<MultiCandle> mCandles;
    };

public:
    TrendFollowStrat();

    ~TrendFollowStrat()
    {
        client->disconnect();
    }

    void runStrat() override;

private:
    int reqsCounter;
    std::vector<Contract> watchList;
    std::vector<Contract> getWatchList();

    void getWatchListHistorical();
    std::map<int, WatchListHistorical> watchListHistoricals;
    int watchListHistoricalDataCount{0};

signals:
    void receivedAllHistoricalData();

public slots:
    virtual void onSignalHistoricalDataBarEndData(int reqId, std::vector<Bar> bars) override;


//signals from within class
    void onReceivedAllHistoricalData();
};

#endif // TRENDFOLLOWSTRAT_H
