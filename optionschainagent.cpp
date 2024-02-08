#include "optionschainagent.h"
#include <thread>

OptionsChainAgent::OptionsChainAgent()
{
    client = new TwsClientQThreaded("127.0.0.1", 7497, "DU8491376", AgentClientIds::OptionsChainClient);
    connectSlots();
    client->connect();
    client->start();
}

void OptionsChainAgent::onSignalGetOptionsChain(std::vector<Contract> contractsVec)
{
    clearContracts();
    for(const auto& con : contractsVec)
    {
        SuperContract scontract;
        scontract.contract = con;
        scontract.reqId = reqIdCounter;
        contracts[reqIdCounter] = scontract;
        reqIdCounter += 1;
    }
    getOptionChains();
}

void OptionsChainAgent::registerContract(Contract contract)
{
    SuperContract scontract;
    scontract.contract = contract;
    scontract.reqId = reqIdCounter;
    contracts[reqIdCounter] = scontract;
    reqIdCounter += 1;
}

void OptionsChainAgent::getOptionChains()
{
    for(const auto& [reqId, scontract] : contracts)
    {
        auto msg = fmtlog(logger, "%s: OptionsChainAgent:reqSecDefOptParams reqId %d %s", __func__,
            reqId,
            scontract.contract.symbol.c_str());
        qDebug() << msg << "\n";
        client->reqSecDefOptParams(reqId, scontract.contract.symbol,
            "" , scontract.contract.secType,scontract.contract.conId);
    }
}

void OptionsChainAgent::clearContracts()
{
    contracts.clear();
    reqIdCounter = 5000;
    responsesCount = 0;
    optionsDetailsCounter = 0;
    contractDetailsResponsesCount = 0;
}

void OptionsChainAgent::onSignalSecurityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId,
                                                 const std::string& tradingClass, const std::string& multiplier, const std::set<std::string>& expirations,
                                                 const std::set<double>& strikes)
{
    if(exchange == "SMART")
    {
        //qDebug() << "optionschhainagent onSignalSecurityDefinitionOptionalParameter\n";
        for(const auto& exp : expirations)
        {
            contracts[reqId].options.calls[exp] = std::vector<OptionsContract>();
            contracts[reqId].options.puts[exp] = std::vector<OptionsContract>();
            contracts[reqId].options.strikes[exp] = std::vector<double>();
            for(const auto& strike : strikes)
            {
                OptionsContract callOption;
                callOption.symbol = contracts[reqId].contract.symbol;
                callOption.strike = strike;
                callOption.lastTradeDateOrContractMonth = exp;
                callOption.right = "C";
                callOption.exchange = "SMART";
                callOption.currency = "USD";
                callOption.secType = "OPT";
                contracts[reqId].options.calls[exp].push_back(callOption);

                OptionsContract putOption;
                putOption.symbol = contracts[reqId].contract.symbol;
                putOption.strike = strike;
                putOption.lastTradeDateOrContractMonth = exp;
                putOption.right = "P";
                putOption.exchange = "SMART";
                putOption.currency = "USD";
                putOption.secType = "OPT";
                contracts[reqId].options.puts[exp].push_back(putOption);
                //contracts[reqId].options.strikes[exp].push_back(strike);
            }
            auto msg = fmtlog(logger, "%s: OptionsChainAgent::onSignalSecurityDefinitionOptionalParameter, "
                                  "Symb: %s Exp: %s has %d put strikes, %d call strikes initially", __func__,
                contracts[reqId].contract.symbol.c_str(), exp.c_str(),
                contracts[reqId].options.puts[exp].size(),
                contracts[reqId].options.calls[exp].size());
            emit signalPassLogMsg(msg);
        }
    }
}

void OptionsChainAgent::onSignalSecurityDefinitionOptionalParameterEnd(int reqId)
{
    //qDebug() << "optionschhainagent onSignalSecurityDefinitionOptionalParameterEnd\n";

    for(auto& [exp, callsVec] : contracts[reqId].options.calls)
    {
        std::sort(std::begin(callsVec), std::end(callsVec),
                [](OptionsContract a, OptionsContract b)
                {
                    return a.strike < b.strike;
                });
    }
    for(auto& [exp, putsVec] : contracts[reqId].options.puts)
    {
        std::sort(std::begin(putsVec), std::end(putsVec),
                [](OptionsContract a, OptionsContract b)
                {
                    return a.strike < b.strike;
                });
    }
    responsesCount += 1;

    if(responsesCount == contracts.size())
    {
        checkOptionsValidity();
    }
}

void OptionsChainAgent::checkOptionsValidity()
{
    //qDebug() << "optionschhainagent checkOptionsValidity\n";
    for(auto& scontract : contracts)
    {
        for(auto& [exp, callsVec] : scontract.second.options.calls)
        {
            for(auto& callOption : callsVec)
            {
                callOption.reqId = reqIdCounter;
                client->reqContractDetails(callOption.reqId, callOption);
                reqIdCounter += 1;
                optionsDetailsCounter += 1;
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(50ms);
            }
        }
        for(auto& [exp, putsVec] : scontract.second.options.puts)
        {
            for(auto& putOption : putsVec)
            {
                putOption.reqId = reqIdCounter;
                client->reqContractDetails(putOption.reqId, putOption);
                reqIdCounter += 1;
                optionsDetailsCounter += 1;
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(50ms);
            }
        }
    }
    auto msg = fmtlog(logger, "%s: OptionsChainAgent::checkOptionsValidity with %d reqs", __func__,
        optionsDetailsCounter);
    emit signalPassLogMsg(msg);
}

void OptionsChainAgent::onSignalContractDetails(int reqid, ContractDetails details)
{
    //qDebug() << "optionschhainagent onSignalContractDetails\n";
}

void OptionsChainAgent::onSignalContractDetailsEnd(int reqid)
{
    //qDebug() << "optionschhainagent onSignalContractDetailsEnd " << contractDetailsResponsesCount << "\n";
    contractDetailsResponsesCount += 1;
    if(optionsDetailsCounter == contractDetailsResponsesCount)
    {
        optionsDetailsCounter = 0;//reset so we dont send multiple
        sendSuperContracts();
    }
}

void OptionsChainAgent::onSignalError(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson)
{
    if(code == 200) //no security definition found
    {
        for(auto& scontract : contracts)
        {
            for(auto& [exp, callsVec] : scontract.second.options.calls)
            {
                for(auto it = callsVec.begin(); it != callsVec.end(); ++it)
                {
                    if(it->reqId == id)
                    {
                        contractDetailsResponsesCount += 1;
                        callsVec.erase(it);
                        break;
                    }
                }
            }
            for(auto& [exp, putsVec] : scontract.second.options.puts)
            {
                for(auto it = putsVec.begin(); it != putsVec.end(); ++it)
                {
                    if(it->reqId == id)
                    {
                        contractDetailsResponsesCount += 1;
                        putsVec.erase(it);
                        break;
                    }
                }
            }
        }
        //qDebug() << id << "optionschhainagent onSignalError 200 " << contractDetailsResponsesCount << "\n";
        if(optionsDetailsCounter == contractDetailsResponsesCount)
        {
            optionsDetailsCounter = 0; //reset
            sendSuperContracts();
        }
    }
}

void OptionsChainAgent::sendSuperContracts()
{
    //qDebug() << "optionschhainagent sendSuperContracts\n";
    emit signalPassLogMsg(fmtlog(logger, "%s: OptionsChainAgent::sendSuperContracts: finished processing options chains", __func__));
    std::vector<SuperContract> contractsVec;
    for(const auto& [reqid, contract] : contracts)
    {
        contractsVec.push_back(contract);
    }
    emit signalSendSuperContracts(contractsVec);
}
