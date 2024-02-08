#ifndef MARKETDATASINGLETON_H
#define MARKETDATASINGLETON_H

#include <mutex>
#include "EWrapper.h"

struct MktData
{
    MktData()
    {
        for(int type = TickType::BID_SIZE; type < TickType::NOT_SET; type++)
        {
            tickTypes[static_cast<TickType>(type)] = std::numeric_limits<double>::min();
        }
    }
    std::map<TickType, double> tickTypes;
};

class MarketDataSingleton
{

public:
    MarketDataSingleton(){}
    ~MarketDataSingleton(){}
    /**
     * Singletons should not be cloneable.
     */
    MarketDataSingleton(MarketDataSingleton &other) = delete;
    /**
     * Singletons should not be assignable.
     */
    void operator=(const MarketDataSingleton &) = delete;
    /**
     * This is the static method that controls the access to the singleton
     * instance. On the first run, it creates a singleton object and places it
     * into the static field. On subsequent runs, it returns the client existing
     * object stored in the static field.
     */
    static MarketDataSingleton* GetInstance();

//data accessors
    void updateMkData(std::string symbol, TickType field, double price)
    {
        //mutex_.lock();
        std::lock_guard<std::mutex> lock(mutex_);
        if(mktDataMap.find(symbol) == mktDataMap.end())
        {
            mktDataMap[symbol] = MktData();
            mktDataMap[symbol].tickTypes[field] = price;
        }
        else
        {
            mktDataMap[symbol].tickTypes[field] = price;
        }

        //mutex_.unlock();
    }

    double getMktData(std::string symbol, TickType field)
    {
        //mutex_.lock();
        std::lock_guard<std::mutex> lock(mutex_);
        double price = mktDataMap[symbol].tickTypes[field];
        //mutex_.unlock();
        return price;
    }

//data structs
    std::map<std::string, MktData> mktDataMap;
private:

    static MarketDataSingleton* singleton_;
    static std::mutex mutex_;
};

#endif // MARKETDATASINGLETON_H
