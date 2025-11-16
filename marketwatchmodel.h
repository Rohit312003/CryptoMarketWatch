/******************************************************************************
 * MarketWatchModel.h
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Description:
 *   Model class for the Market Watch table. Handles row data, updates from
 *   tickers, and UI color change timers.
 *   Inherits from QAbstractTableModel for use with QTableView.
 *   Supports batch row add and mapped row storage for deletion by UID.
 ******************************************************************************/

#ifndef MARKETWATCHMODEL_H
#define MARKETWATCHMODEL_H

#include <QAbstractTableModel>
#include <map>
#include <QTimer>
#include "protocol.h"

extern const QStringList  marketwatchColumnInfo; ///< Global column label info

/**
 * @class MarketWatchModel
 * @brief Table model for market data and UI, supports row broadcasts and color timers.
 */
class MarketWatchModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MarketWatchModel(QObject *parent = nullptr);
    ~MarketWatchModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void addRow(const CryptoCV::MarketWatchRowData &Data);
    void addRows(const QVector<CryptoCV::MarketWatchRowData> &Data);

    /**
     * Removes the instrument row at the given row index from the model.
     * @param row Row index to remove (sequential, not UID)
     */
    void removeRowAt(int row);

    std::map<int, CryptoCV::MarketWatchRowData> rows;

public slots:
    void onBrodcastRcv(CryptoCV::OkxTicker Tick);

private:
    static int uid;
    QMap<int, QTimer*> colorTimers;
};

#endif // MARKETWATCHMODEL_H
