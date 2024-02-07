#include "ordersmanager.h"

OrdersManager::OrdersManager() {}

void OrdersManager::onReceivePlaceOrder(Contract contract, Order order)
{
    mutex.lock();
    ordersQueue.push(OrderStruct{contract, order});
    mutex.unlock();
}

void OrdersManager::onSignalNextValidId(OrderId id)
{
    currOrderId = id;
}


void OrdersManager::run()
{

}
