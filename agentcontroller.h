#ifndef AGENTCONTROLLER_H
#define AGENTCONTROLLER_H

#include <QThread>
#include <QObject>
#include "earningsvolatilitystrat.h"
#include "accountmanager.h"
#include "optionschainagent.h"
#include "databrokeragent.h"
#include "trendfollowstrat.h"

class AgentController : public QObject
{
    Q_OBJECT

    QThread workerThread;
    IBaseAgent* agent;
public:
    AgentController(StrategyAgents agentType)
    {
        switch(agentType)
        {
            case AccountManagerAgent:
            {
                agent = new AccountManager();
                break;
            }
            case EarningsVolatilityAgent:
            {
                agent = new EarningsVolatilityStrat();
                break;
            }
            case OptionsChainRetreiver:
            {
                agent = new OptionsChainAgent();
                break;
            }
            case DataBrokerAgent:
            {
                agent = new DataBroker();
                break;
            }
            case TrendFollowAgent:
            {
                agent = new TrendFollowStrat();
                break;
            }
            default:
            {
                throw std::runtime_error("AgentController: Invalid agent type");
                break;
            }
        }
        agent->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, agent, &QObject::deleteLater);
        connect(&workerThread, &QThread::started, agent, &IBaseAgent::runStrat);
    }

    IBaseAgent* getAgent()
    {
        return agent;
    }

    void start()
    {
        workerThread.start();
    }

    ~AgentController()
    {
        workerThread.quit();
        workerThread.wait();
    }

public slots:
    //void handleResults(const QString &);
signals:
    //void operate(const QString &);
};

#endif // AGENTCONTROLLER_H
