/******************************************************************************
 * MarketWatchDockWindow.h
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Description:
 *   Market Watch dock window UI classes for Crypto Trading Platform.
 *   - Data table for market instruments and prices
 *   - Proxy for sorting/filtering
 *   - Combo/filter controls for instruments
 *   - Handles double-click events for detailed row view
 *   - Saves and loads watched crypto rows and column visibility from INI file
 ******************************************************************************/

#ifndef MARKETWATCHDOCKWINDOW_H
#define MARKETWATCHDOCKWINDOW_H

#include <QDockWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QVector>
#include <QLineEdit>
#include <QJsonObject>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QSettings>
#include <QShortcut>
#include <QKeySequence>
#include "protocol.h"
#include "marketwatchmodel.h"

/**
 * @class MarketWatchDataBase
 * @brief Data storage/model class for market watch instruments and state.
 */
class MarketWatchDataBase
{
public:
    MarketWatchDataBase();
};

/**
 * @class MarketWatchDataTable
 * @brief Main table view for displaying market instrument prices and info.
 */
class MarketWatchDataTable : public QTableView
{
    Q_OBJECT
public:
    explicit MarketWatchDataTable(QWidget *parent = nullptr);

public slots:
    void onHeaderContextMenuRequested(const QPoint &pos);
    void showHideColumnsMenuShortcut();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
signals:
    void columnhideSignal();
    void deleteRowRequested(int row);

};

/**
 * @class marketWatchDockWindow
 * @brief Main dockable window for market watch, table, and controls.
 */
class marketWatchDockWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit marketWatchDockWindow(QWidget *parent = nullptr);

    // Public members for table/proxy/model UI access
    MarketWatchDataTable *table;
    QSortFilterProxyModel *proxy;
    MarketWatchModel *model;

public slots:
    void onTableDoubleClickedResponse(QJsonObject data);
    void loadCryptoRowsFromIni();              // Loads watched symbol list

private slots:
    void onTableDoubleClicked(const QModelIndex &index);
    void onSymbolSelected(const QString &symbol);
    void onAddButtonClicked();
    void deleteRowByIndex(int sourceRow);
private:
    QComboBox *symbolCombo;
    QPushButton *addButton;
    QString OrderBookReqSymbol;                // Symbol for orderbook dialog
    QVector<QLineEdit*> filterEdits;           // Inline filter controls

    // ----- Persistence -----
    void saveColumnVisibilityToIni();          // Saves column show/hide
    void loadColumnVisibilityFromIni();        // Loads column show/hide

    void saveCryptoRowsToIni();                // Saves watched symbol list

};

#endif // MARKETWATCHDOCKWINDOW_H
