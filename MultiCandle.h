#ifndef MULTICANDLE_H
#define MULTICANDLE_H

#include "bar.h"
#include <any>
#include <map>
#include <stdexcept>
#include "IndicatorTypes.h"
#include "timefuncs.h"

class MultiCandle : public Bar
{
public:
    MultiCandle()
    {
    }
    MultiCandle(Bar candle) //default daily tws format
    {
        timestamp = stringTimeToUnix(candle.time, "%Y%m%d");
        time = candle.time;
        low = candle.low;
        high = candle.high;
        open = candle.open;
        close = candle.close;
        volume = candle.volume;
        wap = candle.wap;
        count = candle.count;
    }

    uint64_t getTimestamp() const
    {
        return timestamp;
    }

    std::vector<IndicatorType> listIndicators() const
    {
        std::vector<IndicatorType> l;
        for (const auto& it : indicators)
        {
            l.push_back(it.first);
        }
        return l;
    }

    void setInd(IndicatorType type, IndicatorValue data)
    {
        indicators[type] = data;
    }

    IndicatorValue getInd(IndicatorType type)
    {
        auto it = indicators.find(type);
        if (it == indicators.end())
        {
            std::string msg = "getInd() indicatorname: " + type.name + " not found in multicandle\n";
            throw std::runtime_error(msg.c_str());
        }

        return std::any_cast<IndicatorValue>(it->second);
    }

    template <typename T>
    T getIndByName(std::string name)
    {
        for (const auto& it : indicators)
        {
            if (it.first.name == name)
            {
                return std::any_cast<T>(it.second);
            }
        }
        std::string msg = "getIndByName() indicatorname: " + name + " not found in multicandle\n";
        throw std::runtime_error(msg.c_str());
    }

    std::any getAnyInd(IndicatorType type) const
    {
        auto it = indicators.find(type);
        if (it == indicators.end())
        {
            std::string msg = "getInd() indicatorname: " + type.name + " not found in multicandle\n";
            throw std::runtime_error(msg.c_str());
        }
        return it->second;
    }


private:
    uint64_t timestamp;
    std::map<IndicatorType, std::any> indicators;
};

#endif // MULTICANDLE_H
