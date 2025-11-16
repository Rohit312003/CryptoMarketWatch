/******************************************************************************
 * MarketWatchDockWindow.cpp
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Implements MarketWatch Dock and Table, including:
 * - context menu for column show/hide and row delete
 * - column visibility persistence (.ini)
 * - watched symbol list persistence (.ini)
 * - shortcut/filter controls
 * - add/delete/restore row functionality
 ******************************************************************************/

#include "marketwatchdockwindow.h"
#include "configmanager.h"
#include "websocketconnection.h"
#include "orderbookwindow.h"
#include "globals.h"
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QHeaderView>
#include <QShortcut>
#include <QMenu>
#include <QTimer>

/*---------------------------------------------------------------------------
 * MarketWatchDataBase implementation
 *--------------------------------------------------------------------------*/
MarketWatchDataBase::MarketWatchDataBase() {}

/*---------------------------------------------------------------------------
 * MarketWatchDataTable implementation
 *--------------------------------------------------------------------------*/
MarketWatchDataTable::MarketWatchDataTable(QWidget *parent)
    : QTableView(parent)
{
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &MarketWatchDataTable::onHeaderContextMenuRequested);

    // Ctrl+H shortcut for columns menu
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this);
    connect(shortcut, &QShortcut::activated, this, &MarketWatchDataTable::showHideColumnsMenuShortcut);
}

void MarketWatchDataTable::showHideColumnsMenuShortcut()
{
    QHeaderView *header = horizontalHeader();
    int x = header->width() / 2;
    int y = header->height() / 2;
    QPoint headerPos = header->mapToGlobal(QPoint(x, y));
    onHeaderContextMenuRequested(header->mapFromGlobal(headerPos));
}

// Header context menu for columns show/hide (persists INI)
void MarketWatchDataTable::onHeaderContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QMenu *hideColumnsMenu = menu.addMenu("Hide Columns");
    QAbstractItemModel *tableModel = model();
    if (!tableModel)
        return;
    int colCount = tableModel->columnCount();
    for (int i = 0; i < colCount; ++i) {
        QString headerName = tableModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        QAction *action = hideColumnsMenu->addAction(headerName);
        action->setCheckable(true);
        action->setChecked(isColumnHidden(i));
        connect(action, &QAction::toggled, this, [this, i](bool checked) {
            setColumnHidden(i, checked);
            emit columnhideSignal();
        });
    }
    menu.exec(horizontalHeader()->mapToGlobal(pos));
}

// Right-click context menu for profile and row delete
void MarketWatchDataTable::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) return;
    int row = index.row();
    int col = index.column();
    QMenu menu(this);

    QAction *profileAction = menu.addAction("Show Column Profile");
    connect(profileAction, &QAction::triggered, this, [this, row, col]() {
        QVariant data = model()->data(model()->index(row, col), Qt::DisplayRole);
        QMessageBox::information(this, "Column Profile",
                                 QString("Row %1, Column %2\nValue: %3").arg(row).arg(col).arg(data.toString()));
    });

    QAction *deleteAction = menu.addAction("Delete Row");
    connect(deleteAction, &QAction::triggered, this, [this, row]() {
        emit deleteRowRequested(row);
    });

    menu.exec(event->globalPos());
}

/*---------------------------------------------------------------------------
 * marketWatchDockWindow implementation
 *--------------------------------------------------------------------------*/
marketWatchDockWindow::marketWatchDockWindow(QWidget *parent)
    : QDockWidget("Market Watch", parent)
{
    //--- Header UI setup ---
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background: #1e1e1e; color: white; padding: 5px;");
    QVBoxLayout *mainLayout = new QVBoxLayout(headerWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *title = new QLabel("MarketWatch", headerWidget);
    title->setStyleSheet("font-weight: bold; font-size: 14px; color: white;");
    titleLayout->addWidget(title);
    titleLayout->addStretch();
    mainLayout->addLayout(titleLayout);

    QHBoxLayout *selectionLayout = new QHBoxLayout();
    QLabel *symbolLabel = new QLabel("Select Symbol:", headerWidget);
    symbolLabel->setStyleSheet("color: white; font-size: 12px;");
    symbolCombo = new QComboBox(headerWidget);
    symbolCombo->setStyleSheet(
        "QComboBox { background-color: #2d2d2d; color: white; border: 1px solid #555; padding: 3px; }"
        );
    addButton = new QPushButton("Add", headerWidget);
    addButton->setStyleSheet(
        "QPushButton { background-color: #0078d4; color: white; border: none; padding: 5px 15px; border-radius: 3px; }"
        );
    addButton->setMaximumWidth(60);

    selectionLayout->addWidget(symbolLabel);
    selectionLayout->addWidget(symbolCombo, 1);
    selectionLayout->addWidget(addButton);
    mainLayout->addLayout(selectionLayout);

    headerWidget->setLayout(mainLayout);
    setTitleBarWidget(headerWidget);

    //-- Main table/model setup --
    model = new MarketWatchModel(this);
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    table = new MarketWatchDataTable(this);
    table->setModel(proxy);
    table->setSortingEnabled(true);
    table->horizontalHeader()->setSectionsMovable(true);
    int colCount = model->columnCount();

    //--- Filter row setup ---
    QWidget *filterWidget = new QWidget(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(1, 1, 1, 1);
    filterEdits.resize(colCount);

    for (int col = CryptoCV::MarketWatchColumn::MarketWatch_UID; col < CryptoCV::MarketWatchColumn::MarketWatch_TOTAL_COLUMNS; ++col) {
        if (col == CryptoCV::MarketWatchColumn::MarketWatch_UID || col == CryptoCV::MarketWatchColumn::MarketWatch_SYMBOL) {
            QLineEdit *edit = new QLineEdit(filterWidget);
            edit->setPlaceholderText(model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString());
            filterLayout->addWidget(edit);
            filterEdits[col] = edit;
            connect(edit, &QLineEdit::textChanged, this, [this, col](const QString &text) {
                proxy->setFilterKeyColumn(col);
                proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
                proxy->setFilterFixedString(text);
            });
        }
    }
    filterWidget->setLayout(filterLayout);

    //--- Layout
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(filterWidget);
    vbox->addWidget(table);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(vbox);
    setWidget(centralWidget);

    //--- Favorites combo
    QStringList favoritePairs = ConfigManager::instance().getFavoritePairs();
    symbolCombo->addItems(favoritePairs);
    connect(symbolCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &marketWatchDockWindow::onSymbolSelected);
    connect(addButton, &QPushButton::clicked,
            this, &marketWatchDockWindow::onAddButtonClicked);
    connect(table, &QTableView::doubleClicked,
            this, &marketWatchDockWindow::onTableDoubleClicked);

    connect(table, &MarketWatchDataTable::columnhideSignal, this, &marketWatchDockWindow::saveColumnVisibilityToIni);
    connect(table, &MarketWatchDataTable::deleteRowRequested, this, [this](int proxyRow) {
        QModelIndex proxyIndex = proxy->index(proxyRow, 0);
        QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
        if (sourceIndex.isValid())
            deleteRowByIndex(sourceIndex.row());
    });

    //--- Persistent config
    loadColumnVisibilityFromIni();
}

//-----------------------------------------
// Feature Implementations
//-----------------------------------------
void marketWatchDockWindow::onTableDoubleClickedResponse(QJsonObject data)
{
    OrderBookLevel5 book = OrderBookLevel5::fromJson(data);
    OrderBookWindow *dlg = new OrderBookWindow(book, this, OrderBookReqSymbol);
    dlg->exec();
}

void marketWatchDockWindow::onTableDoubleClicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxy->mapToSource(index);
    int uid = model->data(model->index(sourceIndex.row(), CryptoCV::MarketWatchColumn::MarketWatch_UID), Qt::DisplayRole).toInt();
    extern WebSocketConnection* WEB_SOCKET_CONNECTION;
    int modelIdx = sourceIndex.row();
    auto it = model->rows.begin();
    std::advance(it, modelIdx);
    OrderBookReqSymbol = it->second.symbol;
    WEB_SOCKET_CONNECTION->makeApiRequest(CryptoCV::ApiRequestType::OrderBookSnapshot, OrderBookReqSymbol, 5);
}

void marketWatchDockWindow::onSymbolSelected(const QString &symbol)
{
    qDebug() << "Symbol selected:" << symbol;
}

void marketWatchDockWindow::onAddButtonClicked()
{
    QString symbol = symbolCombo->currentText();
    if (symbol.isEmpty()) return;
    qDebug() << "Adding symbol to table:" << symbol;

    CryptoCV::MarketWatchRowData row;
    row.symbol = symbol;
    row.lastPrice = 0;
    row.bidPrice = 0;
    row.askPrice = 0;
    row.bidQty = 0;
    row.askQty = 0;
    model->addRow(row);
    saveCryptoRowsToIni();
}

//------------------ INI SAVE/LOAD: Columns ------------------
void marketWatchDockWindow::saveColumnVisibilityToIni()
{
    QSettings settings(ConfigManager::instance().configPath(), QSettings::IniFormat);
    settings.beginGroup("MarketWatch/ColumnVisibility");
    int colCount = table->model()->columnCount();
    for (int i = 0; i < colCount; ++i)
        settings.setValue(QString::number(i), table->isColumnHidden(i));
    settings.endGroup();
    settings.sync();
    qDebug() << "Saved column visibility";
}

void marketWatchDockWindow::loadColumnVisibilityFromIni()
{
    QSettings settings(ConfigManager::instance().configPath(), QSettings::IniFormat);
    settings.beginGroup("MarketWatch/ColumnVisibility");
    int colCount = table->model()->columnCount();
    for (int i = 0; i < colCount; ++i) {
        bool hidden = settings.value(QString::number(i), false).toBool();
        table->setColumnHidden(i, hidden);
    }
    settings.endGroup();
}

//------------------ INI SAVE/LOAD: Watched Crypto Rows ------------------
void marketWatchDockWindow::saveCryptoRowsToIni()
{
    QSettings settings(ConfigManager::instance().configPath(), QSettings::IniFormat);
    settings.beginGroup("MarketWatch/CryptoRows");
    settings.remove("");
    QStringList symbolList;
    for (const auto& kv : model->rows) {
        symbolList << kv.second.symbol;
    }
    settings.setValue("symbols", symbolList);
    settings.endGroup();
    settings.sync();
}

void marketWatchDockWindow::loadCryptoRowsFromIni()
{
    QSettings settings(ConfigManager::instance().configPath(), QSettings::IniFormat);
    settings.beginGroup("MarketWatch/CryptoRows");
    QStringList symbolList = settings.value("symbols").toStringList();
    settings.endGroup();
    if (symbolList.isEmpty())
        return;
    int *rowIdx = new int(0);
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, symbolList, rowIdx, timer]() mutable {
        if (*rowIdx < symbolList.size()) {
            const QString& symbol = symbolList[*rowIdx];
            CryptoCV::MarketWatchRowData row;
            row.symbol = symbol;
            row.lastPrice = 0;
            row.bidPrice = 0;
            row.askPrice = 0;
            row.bidQty = 0;
            row.askQty = 0;
            model->addRow(row);
            (*rowIdx)++;
        } else {
            timer->stop();
            timer->deleteLater();
            delete rowIdx;
        }
    });
    timer->start(100);
}

//------------------ Delete Row Logic ------------------
void marketWatchDockWindow::deleteRowByIndex(int sourceRow)
{
    model->removeRowAt(sourceRow); // Use the model's method from earlier
    saveCryptoRowsToIni();
}
