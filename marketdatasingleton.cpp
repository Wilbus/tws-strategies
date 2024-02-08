#include "marketdatasingleton.h"

MarketDataSingleton* MarketDataSingleton::singleton_{nullptr};
std::mutex MarketDataSingleton::mutex_;

MarketDataSingleton* MarketDataSingleton::GetInstance()
{
    if(singleton_ == nullptr)
    {
        singleton_ = new MarketDataSingleton();
    }
    return singleton_;
}
