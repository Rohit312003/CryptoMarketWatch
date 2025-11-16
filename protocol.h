/******************************************************************************
 *  protocol.h
 *  Author: Rohit Kumar
 *  Contact: rohit312003@gmail.com
 *  Date: 15-11-2025
 *
 *  Description:
 *    - Data structures and enums for the Crypto Market Watch application.
 *    - Used for market watch table columns, API request dispatching, and data model.
 ******************************************************************************/

#ifndef PROTOCOL_H
#define PROTOCOL_H
#pragma once

#include <QString>

namespace CryptoCV {

/**
 * @enum MarketWatchColumn
 * @brief Columns for the market watch table (QTableView).
 */
enum MarketWatchColumn
{
    MarketWatch_UID = 0,         ///< Unique row/user ID
    MarketWatch_SYMBOL,          ///< Symbol (e.g. "BTC-USDT")
    MarketWatch_LAST_PRICE,      ///< Last traded price
    MarketWatch_BID_PRICE,       ///< Current bid price
    MarketWatch_ASK_PRICE,       ///< Current ask price
    MarketWatch_ASK_QUANTITY,    ///< Quantity at top ask price
    MarketWatch_BID_QUANTITY,    ///< Quantity at top bid price
    MarketWatch_TOTAL_COLUMNS    ///< Total columns count (for table setup)
};

/**
 * @enum ApiRequestType
 * @brief Enum for dispatching different snapshot requests to OKX REST API.
 */
enum class ApiRequestType {
    TickerSnapshot = 0,      ///< Basic instrument stats
    OrderBookSnapshot,       ///< Top N levels of order book
    RecentTrades            ///< Recent trades/tape
};

/**
 * @struct MarketWatchRowData
 * @brief Data structure for holding all values of a row in the market table.
 */
struct MarketWatchRowData {
    int uid;               ///< Row unique ID
    QString symbol;        ///< Instrument symbol
    double lastPrice;      ///< Last traded price
    double prevPrice = 0.0;
    double bidPrice;       ///< Highest bid price
    double prevBid = 0.0;
    double askPrice;       ///< Lowest ask price
    double prevAsk = 0.0;
    double askQty;         ///< Top ask quantity
    double bidQty;         ///< Top bid quantity
    double prevAskQty = 0.0;
    double prevBidQty = 0.0;
};

/**
 * @struct OkxTicker
 * @brief Structure for parsed ticker stats from OKX live tickers or snapshot.
 */
struct OkxTicker {
    QString instId;   ///< Instrument symbol, e.g. "BTC-USDT"
    double last;      ///< Last traded price
    double bid;       ///< Current best bid price
    double ask;       ///< Current best ask price
    double askQty;    ///< Quantity at best ask
    double bidQty;    ///< Quantity at best bid
};

/**
 * @struct OrderBookLevel
 * @brief One order book row/level as returned by the OKX REST API.
 */
struct OrderBookLevel {
    double price;
    double quantity;
    QString meta;    ///< Extra info: could be ignored or used for notes/context
    int orders;      ///< Number of orders at that level
    OrderBookLevel(double p = 0, double q = 0, QString m = QString(), int o = 0)
        : price(p), quantity(q), meta(std::move(m)), orders(o) {}
};

} // namespace CryptoCV

#endif // PROTOCOL_H
