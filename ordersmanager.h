#ifndef ORDERSMANAGER_H
#define ORDERSMANAGER_H

#include <QThread>
#include "twsclientqthreaded.h"
#include <queue>
#include <QMutex>
#include "ReqIdTypes.h"

class OrdersManager : public QThread
{
    Q_OBJECT
public:
    OrdersManager();

    void run() override;

private:
    TwsClientQThreaded* client;
    std::queue<OrderStruct> ordersQueue;
    QMutex mutex;
    OrderId currOrderId;

signals:
//signals to send to gui window
    void signalPassLogMsg(QString msg);

public slots:
    void onSignalNextValidId(OrderId id);


    void onReceivePlaceOrder(Contract contract, Order order);
};

#endif // ORDERSMANAGER_H
