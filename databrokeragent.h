#ifndef DATABROKERAGENT_H
#define DATABROKERAGENT_H

#include "strategies.h"

class DataBroker : public BaseAgent
{
    Q_OBJECT
public:
    DataBroker();

    void runStrat() override
    {

    }
private:
    int reqIdCounter{9000};
    std::map<int, Contract> reqIdMap;

public slots:
    virtual void onSignalTickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib) override;

//request from other agents
    void onSubscribeMktData(Contract contract);
    void onUnsubscribeMktData(Contract contract);
};

#endif // DATABROKERAGENT_H
