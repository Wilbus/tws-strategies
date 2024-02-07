#include "mainwindow.h"
#include "./ui_mainwindow.h"


void MainWindow::setupPendingOrdersTable()
{
    auto pendingOrdersTable = findChild<QTableWidget*>("pendingOrdersTable");
    QList<QString> qlist;
    qlist << "ID" << "Action" << "Symbol" << "SecType" << "OrderType" << "LmtPrice" << "Total Qty" << "Status" << "Filled" << "Remaining" << "AvgFill";
    pendingOrdersTable->setColumnCount(qlist.count());
    pendingOrdersTable->setHorizontalHeaderLabels(qlist);
}


void MainWindow::onSignalOrderStatusDraw(OrderId orderId, const std::string& status, Decimal filled, Decimal remaining,
        double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld,
        double mktCapPrice)
{
    auto pendingOrdersTable = findChild<QTableWidget*>("pendingOrdersTable");
    int findRow = -1;
    for(unsigned i = 0; i < pendingOrdersTable->rowCount(); i++)
    {
        if(pendingOrdersTable->item(i, 0)->text() == QString::number(orderId))
        {
            findRow = i;
            break;
        }
    }

    if(findRow != -1)
    {
        if(status == "Cancelled")
        {
            pendingOrdersTable->removeRow(findRow);
            return;
        }
        //pendingOrdersTable->setItem(findRow, 4, new QTableWidgetItem(QString::number(lastFillPrice)));
        pendingOrdersTable->setItem(findRow, 7, new QTableWidgetItem(QString(status.c_str())));
        pendingOrdersTable->setItem(findRow, 8, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(filled).c_str())));
        pendingOrdersTable->setItem(findRow, 9, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(remaining).c_str())));
        pendingOrdersTable->setItem(findRow, 10, new QTableWidgetItem(QString::number(avgFillPrice)));
    }
    else
    {
        pendingOrdersTable->insertRow(pendingOrdersTable->rowCount());
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 0, new QTableWidgetItem(QString::number(orderId)));
        //pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 4, new QTableWidgetItem(QString::number(lastFillPrice)));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 7, new QTableWidgetItem(QString(status.c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 8, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(filled).c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 9, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(remaining).c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 10, new QTableWidgetItem(QString::number(avgFillPrice)));
    }
}

void MainWindow::onSignalOpenOrderDraw(OrderId orderId, const Contract& contract, const Order& order, const OrderState& state)
{
    auto pendingOrdersTable = findChild<QTableWidget*>("pendingOrdersTable");
    int findRow = -1;
    for(unsigned i = 0; i < pendingOrdersTable->rowCount(); i++)
    {
        if(pendingOrdersTable->item(i, 0)->text() == QString::number(orderId))
        {
            findRow = i;
            break;
        }
    }
    if(findRow != -1)
    {
        pendingOrdersTable->setItem(findRow, 2, new QTableWidgetItem(QString(contract.symbol.c_str())));
        pendingOrdersTable->setItem(findRow, 4, new QTableWidgetItem(QString(order.orderType.c_str())));
        if(pendingOrdersTable->item(findRow, 4)->text() == QString("LMT"))
            pendingOrdersTable->setItem(findRow, 5, new QTableWidgetItem(QString::number(order.lmtPrice)));
        pendingOrdersTable->setItem(findRow, 6, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(order.totalQuantity).c_str())));
    }
    else
    {
        pendingOrdersTable->insertRow(pendingOrdersTable->rowCount());
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 0, new QTableWidgetItem(QString::number(orderId)));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 1, new QTableWidgetItem(QString(order.action.c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 2, new QTableWidgetItem(QString(contract.symbol.c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 3, new QTableWidgetItem(QString(contract.secType.c_str())));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 4, new QTableWidgetItem(QString(order.orderType.c_str())));
        if(pendingOrdersTable->item(pendingOrdersTable->rowCount() - 1, 4)->text() == QString("LMT"))
            pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 5, new QTableWidgetItem(QString::number(order.lmtPrice)));
        pendingOrdersTable->setItem(pendingOrdersTable->rowCount() - 1, 6, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(order.totalQuantity).c_str())));
    }
}

void MainWindow::setupActivePostitionsTable()
{
    QList<QString> qlist;
    qlist << "conID"
          << "Symbol"
          << "Security Type"
          << "AvgCost"
          << "Pos"
          << "Market Price"
          << "Market Value"
          << "unrealizedPNL"
          << "realizedPNL";
    auto table = findChild<QTableWidget*>("activePositionsTable");
    table->setColumnCount(qlist.count());
    table->setHorizontalHeaderLabels(qlist);
}

void MainWindow::onSignalPortfolioUpdatesDraw(const Contract& contract, Decimal position, double marketPrice, double marketValue,
                                  double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName)
{
    auto table = findChild<QTableWidget*>("activePositionsTable");
    int findRow = -1;
    for(unsigned i = 0; i < table->rowCount(); i++)
    {
        if(table->item(i, 0)->text() == QString::number(contract.conId))
        {
            findRow = i;
            break;
        }
    }

    if(findRow != -1)
    {
        table->setItem(findRow, 0, new QTableWidgetItem(QString::number(contract.conId)));
        table->setItem(findRow, 1, new QTableWidgetItem(QString(contract.symbol.c_str())));
        table->setItem(findRow, 2, new QTableWidgetItem(QString(contract.secType.c_str())));
        table->setItem(findRow, 3, new QTableWidgetItem(QString::number(averageCost)));
        table->setItem(findRow, 4, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(position).c_str())));
        table->setItem(findRow, 5, new QTableWidgetItem(QString::number(marketPrice)));
        table->setItem(findRow, 6, new QTableWidgetItem(QString::number(marketValue)));

        double uPnl_percent = (unrealizedPNL / averageCost) * 100.0;
        QString unrealizedPNLText = QString("%1 \n(%2)\%").arg(QString::number(unrealizedPNL), QString::number(uPnl_percent));
        table->setItem(findRow, 7, new QTableWidgetItem(unrealizedPNLText));

        double rPnl_percent = (realizedPNL / averageCost) * 100.0;
        QString realizedPNLText = QString("%1 \n(%2)\%").arg(QString::number(realizedPNL), QString::number(rPnl_percent));
        table->setItem(findRow, 8, new QTableWidgetItem(realizedPNLText));
    }
    else
    {
        auto rowcount = table->rowCount();
        table->insertRow(rowcount);
        table->setItem(rowcount, 0, new QTableWidgetItem(QString::number(contract.conId)));
        table->setItem(rowcount, 1, new QTableWidgetItem(QString(contract.symbol.c_str())));
        table->setItem(rowcount, 2, new QTableWidgetItem(QString(contract.secType.c_str())));
        table->setItem(rowcount, 3, new QTableWidgetItem(QString::number(averageCost)));
        table->setItem(rowcount, 4, new QTableWidgetItem(QString(DecimalFunctions::decimalStringToDisplay(position).c_str())));
        table->setItem(rowcount, 5, new QTableWidgetItem(QString::number(marketPrice)));
        table->setItem(rowcount, 6, new QTableWidgetItem(QString::number(marketValue)));

        double uPnl_percent = (unrealizedPNL / averageCost) * 100.0;
        QString unrealizedPNLText = QString("%1 \n(%2)\%").arg(QString::number(unrealizedPNL), QString::number(uPnl_percent));
        table->setItem(rowcount, 7, new QTableWidgetItem(unrealizedPNLText));

        double rPnl_percent = (realizedPNL / averageCost) * 100.0;
        QString realizedPNLText = QString("%1 \n(%2)\%").arg(QString::number(realizedPNL), QString::number(rPnl_percent));
        table->setItem(rowcount, 8, new QTableWidgetItem(realizedPNLText));
    }
}

void MainWindow::onSignalAccountUpdatesDraw(
        const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName)
{
    auto netLiquidationValueLabel = findChild<QLabel*>("netLiquidationValueLabel");
    //auto currentInitialMarginLabel = findChild<QLabel*>("currentInitialMarginLabel");
    //auto currentMatintenanceMarginLabel = findChild<QLabel*>("currentMatintenanceMarginLabel");
    auto currentAvailableFundsLabel = findChild<QLabel*>("currentAvailableFundsLabel");
    //auto currentExcessLiquidityLabel = findChild<QLabel*>("currentExcessLiquidityLabel");
    auto cashBalanceLabel = findChild<QLabel*>("cashBalanceLabel");
    auto dayTradesRemainingLabel = findChild<QLabel*>("dayTradesRemainingLabel");
    QString formatted;
    if(key == "AvailableFunds")
    {
        formatted = "Available Funds: " + QString(val.c_str());
        currentAvailableFundsLabel->setText(formatted);
    }
    else if(key == "CashBalance")
    {
        formatted = "Buying Power: " + QString(val.c_str());
        cashBalanceLabel->setText(formatted);
    }
    else if(key == "DayTradesRemaining")
    {
        formatted = "Day Trades Remaining: " + QString(val.c_str());
        dayTradesRemainingLabel->setText(formatted);
    }
    /*else if(key == "ExcessLiquidity")
    {
        formatted = "Excess Liquidity: " + QString(val.c_str());
        currentExcessLiquidityLabel->setText(formatted);
    }
    else if(key == "InitMarginReq")
    {
        formatted = "Initial Margin Req: " + QString(val.c_str());
        currentInitialMarginLabel->setText(formatted);
    }
    else if(key == "MaintMarginReq")
    {
        formatted = "Maintenance Margin Req: " + QString(val.c_str());
        currentMatintenanceMarginLabel->setText(formatted);
    }*/
    else if(key == "NetLiquidation")
    {
        formatted = "Net Liquidation: " + QString(val.c_str());
        netLiquidationValueLabel->setText(formatted);
    }
}
