#include "IndicatorTypes.h"

bool operator<(const IndicatorType& i1, const IndicatorType& i2)
{
    return i1.name < i2.name;
}

std::string indicatorTypesToString(IndicatorTypes type)
{
    switch (type)
    {
        case SMA:
            return "sma";
        case EMA:
            return "ema";
        default:
            return "unkowntype";
    }
}
