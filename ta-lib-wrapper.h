#ifndef TALIB_H
#define TALIB_H

#include "IndicatorTypes.h"
#include "MultiCandle.h"
#include "ta-lib/ta_common.h"

#include <vector>

namespace ta_lib_wrapper {

class TALIB
{
public:
    TALIB();
    ~TALIB();

    std::vector<IndicatorValue> copyOutToIndicatorValues(
        const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, TA_Integer outBeg, TA_Real out[]);

    std::vector<IndicatorValue> SMA(const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, int periods);
    std::vector<IndicatorValue> EMA(
        const std::vector<MultiCandle>& mcandles, int startIdx, int endIdx, int periods, int smoothing);
    std::vector<IndicatorValue> BB(const std::vector<MultiCandle>& mcandles, IndicatorTypes bbtype, int startIdx,
        int endIdx, int periods, double devUp, double devDown);
};
} // namespace ta_lib_wrapper

#endif // TALIB_H
