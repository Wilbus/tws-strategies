#ifndef OPTIONSCHAINAGENT_H
#define OPTIONSCHAINAGENT_H

#include "strategies.h"

class OptionsChainAgent : public BaseAgent
{
    Q_OBJECT
public:
    OptionsChainAgent();

    void registerContract(Contract contract);
    void getOptionChains();
    void clearContracts();

    void runStrat() override
    {
        //does nothing
    }

private:
    OptionChain optionsChainBuff;



    int reqIdCounter{5000};
    std::map<int, SuperContract> contracts;
    int responsesCount{0};
    int optionsDetailsCounter{0};
    int contractDetailsResponsesCount{0};
    time_t latestExp;

    void checkOptionsValidity();

    void sendSuperContracts();

signals:
    void signalSendOptionsChain(OptionChain optionsChain);
    void signalSendSuperContracts(std::vector<SuperContract> contracts);


public slots:
    virtual void onSignalError(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson) override;

    virtual void onSignalContractDetails(int reqid, ContractDetails details) override;
    virtual void onSignalContractDetailsEnd(int reqid) override;

    virtual void onSignalSecurityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId,
        const std::string& tradingClass, const std::string& multiplier, const std::set<std::string>& expirations,
        const std::set<double>& strikes) override;
    virtual void onSignalSecurityDefinitionOptionalParameterEnd(int reqId) override;

    void onSignalGetOptionsChain(std::vector<Contract> contracts);
    void onSignalGetOptionsChain(std::vector<Contract> contracts, time_t latestExp);
};

#endif // OPTIONSCHAINAGENT_H
