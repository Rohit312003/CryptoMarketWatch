/******************************************************************************
 * MarketWatchModel.cpp
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Implementation of MarketWatchModel for the Crypto Trading Platform.
 * Handles table display, row updates, color-coding, deletion, and integration with
 * WebSocket tickers (via OkxTicker). Uses std::map for UID-based storage.
 ******************************************************************************/

#include "marketwatchmodel.h"
#include "websocketconnection.h"
#include "globals.h"
#include <QDebug>
#include <QColor>
#include <QTimer>
#include <QMap>
#include <cmath>

// Static unique ID for each row
int MarketWatchModel::uid = 0;

// Column info (displayed in header)
const QStringList marketwatchColumnInfo = {
    "unique_ID",
    "Symbol",
    "Last Price",
    "Bid Price",
    "Ask Price",
    "Ask Quantity",
    "Bid Quantity"
};

// Color flash threshold: any change triggers flash
static const double FLASH_THRESHOLD = 0.00;

//------------------------------------------------------------------------------
// Constructor & Destructor
//------------------------------------------------------------------------------
MarketWatchModel::MarketWatchModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

MarketWatchModel::~MarketWatchModel()
{
    qDeleteAll(colorTimers);
    colorTimers.clear();
}

//------------------------------------------------------------------------------
// Table Model Overrides
//------------------------------------------------------------------------------
int MarketWatchModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(rows.size());
}

int MarketWatchModel::columnCount(const QModelIndex &) const
{
    return CryptoCV::MarketWatchColumn::MarketWatch_TOTAL_COLUMNS;
}

QVariant MarketWatchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // Use iterator math for std::map access by row index
    auto it = rows.cbegin();
    std::advance(it, index.row());
    if (it == rows.cend())
        return QVariant();
    const CryptoCV::MarketWatchRowData &r = it->second;

    // Display data in cells
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case CryptoCV::MarketWatchColumn::MarketWatch_UID:         return r.uid;
        case CryptoCV::MarketWatchColumn::MarketWatch_SYMBOL:      return r.symbol;
        case CryptoCV::MarketWatchColumn::MarketWatch_LAST_PRICE:  return QString::number(r.lastPrice, 'f', 2);
        case CryptoCV::MarketWatchColumn::MarketWatch_BID_PRICE:   return QString::number(r.bidPrice, 'f', 2);
        case CryptoCV::MarketWatchColumn::MarketWatch_ASK_PRICE:   return QString::number(r.askPrice, 'f', 2);
        case CryptoCV::MarketWatchColumn::MarketWatch_ASK_QUANTITY:return QString::number(r.askQty, 'f', 4);
        case CryptoCV::MarketWatchColumn::MarketWatch_BID_QUANTITY:return QString::number(r.bidQty, 'f', 4);
        }
    }
    // Color coding for cell updates
    else if (role == Qt::ForegroundRole) {
        switch (index.column()) {
        case CryptoCV::MarketWatchColumn::MarketWatch_LAST_PRICE:
            if (r.lastPrice > r.prevPrice + FLASH_THRESHOLD) return QColor(Qt::green);
            else if (r.lastPrice < r.prevPrice - FLASH_THRESHOLD) return QColor(Qt::red);
            break;
        case CryptoCV::MarketWatchColumn::MarketWatch_BID_PRICE:
            if (r.bidPrice > r.prevBid + FLASH_THRESHOLD) return QColor(Qt::green);
            else if (r.bidPrice < r.prevBid - FLASH_THRESHOLD) return QColor(Qt::red);
            break;
        case CryptoCV::MarketWatchColumn::MarketWatch_ASK_PRICE:
            if (r.askPrice > r.prevAsk + FLASH_THRESHOLD) return QColor(Qt::green);
            else if (r.askPrice < r.prevAsk - FLASH_THRESHOLD) return QColor(Qt::red);
            break;
        case CryptoCV::MarketWatchColumn::MarketWatch_ASK_QUANTITY:
            if (r.askQty > r.prevAskQty + FLASH_THRESHOLD) return QColor(Qt::green);
            else if (r.askQty < r.prevAskQty - FLASH_THRESHOLD) return QColor(Qt::red);
            break;
        case CryptoCV::MarketWatchColumn::MarketWatch_BID_QUANTITY:
            if (r.bidQty > r.prevBidQty + FLASH_THRESHOLD) return QColor(Qt::green);
            else if (r.bidQty < r.prevBidQty - FLASH_THRESHOLD) return QColor(Qt::red);
            break;
        default:
            break;
        }
    }
    // Center text alignment
    else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant MarketWatchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section >= 0 && section < marketwatchColumnInfo.size())
            return marketwatchColumnInfo[section];
    }
    return QVariant();
}

//------------------------------------------------------------------------------
// Add New Row
//------------------------------------------------------------------------------
void MarketWatchModel::addRow(const CryptoCV::MarketWatchRowData &Data)
{
    int rowIndex = static_cast<int>(rows.size());
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);

    CryptoCV::MarketWatchRowData newRow = Data;
    newRow.uid = ++uid;
    newRow.prevPrice = newRow.lastPrice;
    newRow.prevBid   = newRow.bidPrice;
    newRow.prevAsk   = newRow.askPrice;
    newRow.prevAskQty = newRow.askQty;
    newRow.prevBidQty = newRow.bidQty;
    rows[newRow.uid] = newRow;
    endInsertRows();

    extern WebSocketConnection* WEB_SOCKET_CONNECTION;
    WEB_SOCKET_CONNECTION->subscribeTickers(QStringList() << Data.symbol);
}

void MarketWatchModel::addRows(const QVector<CryptoCV::MarketWatchRowData> &Data)
{
    QStringList listOfSymbol;
    for(const auto& symbolRow : Data) {
        int rowIndex = static_cast<int>(rows.size());
        beginInsertRows(QModelIndex(), rowIndex, rowIndex);
        CryptoCV::MarketWatchRowData newRow = symbolRow;
        newRow.uid = ++uid;
        newRow.prevPrice = newRow.lastPrice;
        newRow.prevBid   = newRow.bidPrice;
        newRow.prevAsk   = newRow.askPrice;
        newRow.prevAskQty = newRow.askQty;
        newRow.prevBidQty = newRow.bidQty;
        rows[newRow.uid] = newRow;
        endInsertRows();
        listOfSymbol << symbolRow.symbol;
    }
    extern WebSocketConnection* WEB_SOCKET_CONNECTION;
    WEB_SOCKET_CONNECTION->subscribeTickers(listOfSymbol);
}


void MarketWatchModel::removeRowAt(int row)
{
    if (row < 0 || row >= static_cast<int>(rows.size()))
        return;
    auto it = rows.begin();
    std::advance(it, row);

    beginRemoveRows(QModelIndex(), row, row);   // Tell the view
    rows.erase(it);                             // Remove from map
    endRemoveRows();                            // Tell the view
}


void MarketWatchModel::onBrodcastRcv(CryptoCV::OkxTicker Tick)
{
    int rowIndex = 0;
    for (const auto &kv : rows) {
        auto& r = const_cast<CryptoCV::MarketWatchRowData&>(kv.second); // modifiable reference
        if (Tick.instId == r.symbol) {
            bool changed = std::fabs(Tick.last - r.lastPrice) > FLASH_THRESHOLD ||
                           std::fabs(Tick.bid  - r.bidPrice) > FLASH_THRESHOLD ||
                           std::fabs(Tick.ask  - r.askPrice) > FLASH_THRESHOLD ||
                           std::fabs(Tick.bidQty - r.bidQty) > FLASH_THRESHOLD ||
                           std::fabs(Tick.askQty - r.askQty) > FLASH_THRESHOLD;
            if (!changed) { ++rowIndex; continue; }

            r.prevPrice = r.lastPrice;
            r.prevBid   = r.bidPrice;
            r.prevAsk   = r.askPrice;
            r.prevAskQty = r.askQty;
            r.prevBidQty = r.bidQty;

            r.lastPrice = Tick.last;
            r.bidPrice  = Tick.bid;
            r.askPrice  = Tick.ask;
            r.bidQty    = Tick.bidQty;
            r.askQty    = Tick.askQty;

            QModelIndex topLeft = index(rowIndex, 0);
            QModelIndex bottomRight = index(rowIndex, columnCount() - 1);
            emit dataChanged(topLeft, bottomRight,
                             {Qt::DisplayRole, Qt::ForegroundRole, Qt::TextAlignmentRole});

            if (colorTimers.contains(kv.first)) {
                colorTimers[kv.first]->stop();
                colorTimers[kv.first]->deleteLater();
            }
            QTimer* timer = new QTimer(this);
            colorTimers[kv.first] = timer;
            timer->setSingleShot(true);

            connect(timer, &QTimer::timeout, this, [this, rowIndex, key=kv.first]() {
                auto it = rows.find(key);
                if (it != rows.end()) {
                    auto &r = it->second;
                    r.prevPrice = r.lastPrice;
                    r.prevBid   = r.bidPrice;
                    r.prevAsk   = r.askPrice;
                    r.prevAskQty = r.askQty;
                    r.prevBidQty = r.bidQty;
                    QModelIndex topLeft = index(rowIndex, 0);
                    QModelIndex bottomRight = index(rowIndex, columnCount() - 1);
                    emit dataChanged(topLeft, bottomRight,
                                     {Qt::ForegroundRole, Qt::TextAlignmentRole});
                }
                colorTimers.remove(key);
                sender()->deleteLater();
            });

            timer->start(150);
        }
        ++rowIndex;
    }
}
