#include "earningsvolatilitystratmonitor.h"

EarningsVolatilityStratMonitor::EarningsVolatilityStratMonitor()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", AgentClientIds::EarningsVolatilityMonitorClient);
    connectSlots();

    client->connect();
    client->start();
}
