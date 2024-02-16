#include "ta-lib-wrapper.h"

#include "ta-lib/ta_abstract.h"
#include "ta-lib/ta_defs.h"
#include "ta-lib/ta_func.h"
#include "ta-lib/ta_libc.h"

namespace ta_lib_wrapper {

TALIB::TALIB()
{
    TA_Initialize();
}

TALIB::~TALIB()
{
    TA_Shutdown();
}

std::vector<IndicatorValue> TALIB::copyOutToIndicatorValues(
    const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, TA_Integer outBeg, TA_Real out[])
{
    std::vector<IndicatorValue> vals;
    for (int i = startIdx; i <= endIdx; i++)
    {
        if (i < outBeg)
        {
            IndicatorValue ivalue;
            ivalue.timestamp = mcandles[i].getTimestamp();
            vals.push_back(ivalue);
        }
        else
        {
            IndicatorValue ivalue;
            ivalue.isNan = false;
            ivalue.value = out[i - outBeg];
            ivalue.timestamp = mcandles[i].getTimestamp();
            vals.push_back(ivalue);
        }
    }
    return vals;
}

std::vector<IndicatorValue> TALIB::SMA(const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, int periods)
{
    TA_Real closePrices[endIdx - startIdx + 1];
    TA_Real out[endIdx - startIdx + 1];
    TA_Integer outBeg;
    TA_Integer outNbElement;

    unsigned idx = 0;
    for (const auto& candle : mcandles)
    {
        closePrices[idx] = candle.close;
        idx++;
    }

    auto retCode = TA_MA(startIdx, endIdx, &closePrices[0], periods, TA_MAType_SMA, &outBeg, &outNbElement, &out[0]);

    if (retCode != 0)
    {
        throw std::runtime_error("Error calculating SMA");
    }

    return copyOutToIndicatorValues(mcandles, startIdx, endIdx, outBeg, out);
}

std::vector<IndicatorValue> TALIB::EMA(
    const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, int periods, int smoothing)
{
    TA_Real closePrices[endIdx - startIdx + 1];
    TA_Real out[endIdx - startIdx + 1];
    TA_Integer outBeg;
    TA_Integer outNbElement;

    unsigned idx = 0;
    for (const auto& candle : mcandles)
    {
        closePrices[idx] = candle.close;
        idx++;
    }

    auto retCode = TA_EMA(startIdx, endIdx, &closePrices[0], periods, &outBeg, &outNbElement, &out[0]);

    if (retCode != 0)
    {
        throw std::runtime_error("Error calculating EMA");
    }

    return copyOutToIndicatorValues(mcandles, startIdx, endIdx, outBeg, out);
}

std::vector<IndicatorValue> TALIB::BB(const std::vector<MultiCandle>& mcandles, IndicatorTypes bbtype, int startIdx,
    int endIdx, int periods, double devUp, double devDown)
{
    TA_Real closePrices[endIdx - startIdx + 1];
    TA_Real outUpper[endIdx - startIdx + 1];
    TA_Real outMid[endIdx - startIdx + 1];
    TA_Real outLower[endIdx - startIdx + 1];
    TA_Integer outBeg;
    TA_Integer outNbElement;

    unsigned idx = 0;
    for (const auto& candle : mcandles)
    {
        closePrices[idx] = candle.close;
        idx++;
    }

    auto retCode = TA_BBANDS(startIdx, endIdx, &closePrices[0], periods, devUp, devDown, TA_MAType_SMA, &outBeg,
        &outNbElement, &outUpper[0], &outMid[0], &outLower[0]);
    if (retCode != 0)
    {
        throw std::runtime_error("Error calculating BB");
    }

    switch (bbtype)
    {
        case IndicatorTypes::BBMID: {
            return copyOutToIndicatorValues(mcandles, startIdx, endIdx, outBeg, outMid);
            break;
        }
        case IndicatorTypes::BBUP: {
            return copyOutToIndicatorValues(mcandles, startIdx, endIdx, outBeg, outUpper);
            break;
        }
        case IndicatorTypes::BBDOWN: {
            return copyOutToIndicatorValues(mcandles, startIdx, endIdx, outBeg, outLower);
            break;
        }
        default: {
            throw std::runtime_error("Invalid bbtype given");
        }
    }
}

} // namespace ta_lib_wrapper
