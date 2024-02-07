#ifndef REQIDTYPES_H
#define REQIDTYPES_H

#include <map>
#include "EWrapper.h"
#include "Order.h"
#include "Contract.h"
#include "OrderState.h"

enum ReqIdType
{
    reqAccountSummary = 0,
    reqAccountUpdates,
    reqPostionUpdates,
    reqScannerSubscription,
    reqContractDetails,
    reqHistoricalData,
    reqRealTimeBars,
    reqTickByTickBidAsk,
    reqWshEventData,
    reqWshMetaData,
};

enum MarketDataTypes
{
    Live = 1,
    Frozen = 2,
    Delayed = 3,
    DelayedFrozen = 4
};

enum ClientIds
{
    REGULAR = 0,
    CHARTING,
};

struct ActivePosition
{
    Contract contract;
    Decimal qtyPos;
    double marketPrice;
    double marketValue;
    double averageCost;
    double unrealPNL;
    double realPNL;
    std::string accountName;
};

struct OrderStruct
{
    Contract contract;
    Order order;
};

struct PendingOrderStruct
{
    Contract contract;
    Order order;
    OrderState state;
    std::string status;
    Decimal filled;
    Decimal remaining;
    double avgFillPrice;
    double permId;
    int parentId;
    double lastFillPrice;
    int clientId;
    std::string whyHeld;
    double mktCapPrice;
};

struct OptionsContract : Contract
{
    int reqId;
};

struct OptionChain
{
    std::map<std::string, std::vector<OptionsContract>> calls;
    std::map<std::string, std::vector<OptionsContract>> puts;
    std::map<std::string, std::vector<double>> strikes;
    //std::map<std::string, std::vector<Contract>> optionsContracts;
};

struct SuperContract
{
    int reqId;
    Contract contract;
    OptionChain options;

    bool operator==(const SuperContract& other)
    {
        return this->contract.symbol == other.contract.symbol;
    }
};

// clang-format off
/*const std::map<TickType, std::string> tickTypeToString =
{
    {TickType::BID_SIZE, "LAST"},
    {TickType::LAST, "LAST"},
    {TickType::DELAYED_BID, "DELAYED_BID"},
    {TickType::DELAYED_ASK, "DELAYED_ASK"},
    {TickType::DELAYED_LAST, "DELAYED_LAST"},
    {TickType::DELAYED_BID_SIZE, "DELAYED_BID_SIZE"},
    {TickType::DELAYED_ASK_SIZE, "DELAYED_ASK_SIZE"},
    {TickType::DELAYED_LAST_SIZE, "DELAYED_LAST_SIZE"},
    {TickType::DELAYED_HIGH, "DELAYED_HIGH"},
    {TickType::DELAYED_LOW, "DELAYED_LOW"},
    {TickType::DELAYED_VOLUME, "DELAYED_VOLUME"},
    {TickType::DELAYED_CLOSE, "DELAYED_CLOSE"},
    {TickType::DELAYED_OPEN, "DELAYED_OPEN"},
};*/

const static std::map<TickType, std::string> tickTypeToString = {
    {BID_SIZE, "BID_SIZE"},
    {BID, "BID"},
    {ASK, "ASK"},
    {ASK_SIZE, "ASK_SIZE"},
    {LAST, "LAST"},
    {LAST_SIZE, "LAST_SIZE"},
    {HIGH, "HIGH"},
    {LOW, "LOW"},
    {VOLUME, "VOLUME"},
    {CLOSE, "CLOSE"},
    {BID_OPTION_COMPUTATION, "BID_OPTION_COMPUTATION"},
    {ASK_OPTION_COMPUTATION, "ASK_OPTION_COMPUTATION"},
    {LAST_OPTION_COMPUTATION, "LAST_OPTION_COMPUTATION"},
    {MODEL_OPTION, "MODEL_OPTION"},
    {OPEN, "OPEN"},
    {LOW_13_WEEK, "LOW_13_WEEK"},
    {HIGH_13_WEEK, "HIGH_13_WEEK"},
    {LOW_26_WEEK, "LOW_26_WEEK"},
    {HIGH_26_WEEK, "HIGH_26_WEEK"},
    {LOW_52_WEEK, "LOW_52_WEEK"},
    {HIGH_52_WEEK, "HIGH_52_WEEK"},
    {AVG_VOLUME, "AVG_VOLUME"},
    {OPEN_INTEREST, "OPEN_INTEREST"},
    {OPTION_HISTORICAL_VOL, "OPTION_HISTORICAL_VOL"},
    {OPTION_IMPLIED_VOL, "OPTION_IMPLIED_VOL"},
    {OPTION_BID_EXCH, "OPTION_BID_EXCH"},
    {OPTION_ASK_EXCH, "OPTION_ASK_EXCH"},
    {OPTION_CALL_OPEN_INTEREST, "OPTION_CALL_OPEN_INTEREST"},
    {OPTION_PUT_OPEN_INTEREST, "OPTION_PUT_OPEN_INTEREST"},
    {OPTION_CALL_VOLUME, "OPTION_CALL_VOLUME"},
    {OPTION_PUT_VOLUME, "OPTION_PUT_VOLUME"},
    {INDEX_FUTURE_PREMIUM, "INDEX_FUTURE_PREMIUM"},
    {BID_EXCH, "BID_EXCH"},
    {ASK_EXCH, "ASK_EXCH"},
    {AUCTION_VOLUME, "AUCTION_VOLUME"},
    {AUCTION_PRICE, "AUCTION_PRICE"},
    {AUCTION_IMBALANCE, "AUCTION_IMBALANCE"},
    {MARK_PRICE, "MARK_PRICE"},
    {BID_EFP_COMPUTATION, "BID_EFP_COMPUTATION"},
    {ASK_EFP_COMPUTATION, "ASK_EFP_COMPUTATION"},
    {LAST_EFP_COMPUTATION, "LAST_EFP_COMPUTATION"},
    {OPEN_EFP_COMPUTATION, "OPEN_EFP_COMPUTATION"},
    {HIGH_EFP_COMPUTATION, "HIGH_EFP_COMPUTATION"},
    {LOW_EFP_COMPUTATION, "LOW_EFP_COMPUTATION"},
    {CLOSE_EFP_COMPUTATION, "CLOSE_EFP_COMPUTATION"},
    {LAST_TIMESTAMP, "LAST_TIMESTAMP"},
    {SHORTABLE, "SHORTABLE"},
    {FUNDAMENTAL_RATIOS, "FUNDAMENTAL_RATIOS"},
    {RT_VOLUME, "RT_VOLUME"},
    {HALTED, "HALTED"},
    {BID_YIELD, "BID_YIELD"},
    {ASK_YIELD, "ASK_YIELD"},
    {LAST_YIELD, "LAST_YIELD"},
    {CUST_OPTION_COMPUTATION, "CUST_OPTION_COMPUTATION"},
    {TRADE_COUNT, "TRADE_COUNT"},
    {TRADE_RATE, "TRADE_RATE"},
    {VOLUME_RATE, "VOLUME_RATE"},
    {LAST_RTH_TRADE, "LAST_RTH_TRADE"},
    {RT_HISTORICAL_VOL, "RT_HISTORICAL_VOL"},
    {IB_DIVIDENDS, "IB_DIVIDENDS"},
    {BOND_FACTOR_MULTIPLIER, "BOND_FACTOR_MULTIPLIER"},
    {REGULATORY_IMBALANCE, "REGULATORY_IMBALANCE"},
    {NEWS_TICK, "NEWS_TICK"},
    {SHORT_TERM_VOLUME_3_MIN, "SHORT_TERM_VOLUME_3_MIN"},
    {SHORT_TERM_VOLUME_5_MIN, "SHORT_TERM_VOLUME_5_MIN"},
    {SHORT_TERM_VOLUME_10_MIN, "SHORT_TERM_VOLUME_10_MIN"},
    {DELAYED_BID, "DELAYED_BID"},
    {DELAYED_ASK, "DELAYED_ASK"},
    {DELAYED_LAST, "DELAYED_LAST"},
    {DELAYED_BID_SIZE, "DELAYED_BID_SIZE"},
    {DELAYED_ASK_SIZE, "DELAYED_ASK_SIZE"},
    {DELAYED_LAST_SIZE, "DELAYED_LAST_SIZE"},
    {DELAYED_HIGH, "DELAYED_HIGH"},
    {DELAYED_LOW, "DELAYED_LOW"},
    {DELAYED_VOLUME, "DELAYED_VOLUME"},
    {DELAYED_CLOSE, "DELAYED_CLOSE"},
    {DELAYED_OPEN, "DELAYED_OPEN"},
    {RT_TRD_VOLUME, "RT_TRD_VOLUME"},
    {CREDITMAN_MARK_PRICE, "CREDITMAN_MARK_PRICE"},
    {CREDITMAN_SLOW_MARK_PRICE, "CREDITMAN_SLOW_MARK_PRICE"},
    {DELAYED_BID_OPTION_COMPUTATION, "DELAYED_BID_OPTION_COMPUTATION"},
    {DELAYED_ASK_OPTION_COMPUTATION, "DELAYED_ASK_OPTION_COMPUTATION"},
    {DELAYED_LAST_OPTION_COMPUTATION, "DELAYED_LAST_OPTION_COMPUTATION"},
    {DELAYED_MODEL_OPTION_COMPUTATION, "DELAYED_MODEL_OPTION_COMPUTATION"},
    {LAST_EXCH, "LAST_EXCH"},
    {LAST_REG_TIME, "LAST_REG_TIME"},
    {FUTURES_OPEN_INTEREST, "FUTURES_OPEN_INTEREST"},
    {AVG_OPT_VOLUME, "AVG_OPT_VOLUME"},
    {DELAYED_LAST_TIMESTAMP, "DELAYED_LAST_TIMESTAMP"},
    {SHORTABLE_SHARES, "SHORTABLE_SHARES"},
    {DELAYED_HALTED, "DELAYED_HALTED"},
    {REUTERS_2_MUTUAL_FUNDS, "REUTERS_2_MUTUAL_FUNDS"},
    {ETF_NAV_CLOSE, "ETF_NAV_CLOSE"},
    {ETF_NAV_PRIOR_CLOSE, "ETF_NAV_PRIOR_CLOSE"},
    {ETF_NAV_BID, "ETF_NAV_BID"},
    {ETF_NAV_ASK, "ETF_NAV_ASK"},
    {ETF_NAV_LAST, "ETF_NAV_LAST"},
    {ETF_FROZEN_NAV_LAST, "ETF_FROZEN_NAV_LAST"},
    {ETF_NAV_HIGH, "ETF_NAV_HIGH"},
    {ETF_NAV_LOW, "ETF_NAV_LOW"},
    {SOCIAL_MARKET_ANALYTICS, "SOCIAL_MARKET_ANALYTICS"},
    {ESTIMATED_IPO_MIDPOINT, "ESTIMATED_IPO_MIDPOINT"},
    {FINAL_IPO_LAST, "FINAL_IPO_LAST"},
    {DELAYED_YIELD_BID, "DELAYED_YIELD_BID"},
    {DELAYED_YIELD_ASK, "DELAYED_YIELD_ASK"},
    {NOT_SET, "NOT_SET"}
};

// clang-format on

const unsigned tickByTickCount = 1000;

#endif // REQIDTYPES_H
