/******************************************************************************
 * OrderBookWindow.h
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Description:
 *   Dialog class for displaying the order book (top N levels) of a crypto instrument.
 *   Includes conversion from JSON, model setup, and UI logic.
 ******************************************************************************/

#ifndef ORDERBOOKWINDOW_H
#define ORDERBOOKWINDOW_H

#include <QDialog>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include "protocol.h"

/**
 * @struct OrderBookLevel5
 * @brief Container for top 5 bid/ask levels in the order book; can parse from QJsonObject.
 */
struct OrderBookLevel5 {
    QVector<CryptoCV::OrderBookLevel> asks; ///< Ask levels (lowest price first)
    QVector<CryptoCV::OrderBookLevel> bids; ///< Bid levels (highest price first)
    qint64 ts = 0;                          ///< Timestamp of snapshot

    /**
     * Static helper to construct OrderBookLevel5 from OKX REST API JSON response.
     * @param obj The QJsonObject containing "asks"/"bids"/"ts"
     * @return Populated OrderBookLevel5
     */
    static OrderBookLevel5 fromJson(const QJsonObject& obj);
};

namespace Ui {
class OrderBookWindow;
}

/**
 * @class OrderBookWindow
 * @brief QDialog window for displaying order book details and ladder for a symbol.
 */
class OrderBookWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructs dialog and populates UI from OrderBookLevel5.
     * @param book  Top 5 levels (bids/asks) of order book
     * @param parent Dialog parent widget
     * @param Symbol Instrument symbol (used for labeling UI)
     */
    explicit OrderBookWindow(const OrderBookLevel5 &book, QWidget *parent = nullptr, QString Symbol = "");

    /**
     * Destructor to clean up UI.
     */
    ~OrderBookWindow();

private:
    Ui::OrderBookWindow *ui; ///< UI pointer for design/import
    /**
     * Sets up order book table/grid in the dialog.
     * @param book OrderBookLevel5 with all bid/ask levels
     */
    void setupOrderBook(const OrderBookLevel5 &book);
};

#endif // ORDERBOOKWINDOW_H
