#include "orderbookwindow.h"
#include "ui_orderbookwindow.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>


OrderBookLevel5 OrderBookLevel5::fromJson(const QJsonObject& obj)
{
    OrderBookLevel5 book;
    auto dataArr = obj.value("data").toArray();
    if (dataArr.isEmpty()) return book;

    QJsonObject data = dataArr.at(0).toObject();
    book.ts = data.value("ts").toVariant().toLongLong();

    QJsonArray asks = data.value("asks").toArray();
    for (const auto& askVal : asks) {
        QJsonArray arr = askVal.toArray();
        book.asks.append(CryptoCV::OrderBookLevel(
            arr.at(0).toString().toDouble(),
            arr.at(1).toString().toDouble(),
            arr.at(2).toString(),
            arr.at(3).toString().toInt()
            ));
    }
    if (book.asks.size() > 5) book.asks.resize(5);

    QJsonArray bids = data.value("bids").toArray();
    for (const auto& bidVal : bids) {
        QJsonArray arr = bidVal.toArray();
        book.bids.append(CryptoCV::OrderBookLevel(
            arr.at(0).toString().toDouble(),
            arr.at(1).toString().toDouble(),
            arr.at(2).toString(),
            arr.at(3).toString().toInt()
            ));
    }
    if (book.bids.size() > 5) book.bids.resize(5);

    return book;
}

OrderBookWindow::OrderBookWindow(const OrderBookLevel5 &book, QWidget *parent,QString Symbol ) :
    QDialog(parent),
    ui(new Ui::OrderBookWindow)
{
    ui->setupUi(this);        // Codec-generated UI setup
    ui->label_2->setText(Symbol);
    setupOrderBook(book);     // Fill the tableView with your orderbook
    setWindowTitle("Order Book");
}

OrderBookWindow::~OrderBookWindow()
{
    delete ui;
}


void OrderBookWindow::setupOrderBook(const OrderBookLevel5 &book)
{
    QStandardItemModel *model = new QStandardItemModel(this);


    // Sort asks by ascending price
    QVector<CryptoCV::OrderBookLevel> asks = book.asks;
    std::sort(asks.begin(), asks.end(), [](const CryptoCV::OrderBookLevel &a, const CryptoCV::OrderBookLevel &b){
        return a.price < b.price; // ascending
    });

    // Sort bids by descending price
    QVector<CryptoCV::OrderBookLevel> bids = book.bids;
    std::sort(bids.begin(), bids.end(), [](const CryptoCV::OrderBookLevel &a, const CryptoCV::OrderBookLevel &b){
        return a.price > b.price; // descending
    });

    // --- ASKS TABLE ---
    QStandardItemModel *asksModel = new QStandardItemModel(this);
    asksModel->setColumnCount(2);
    asksModel->setHorizontalHeaderLabels({"Ask Price", "Ask Qty"});

    for (const auto &lvl : asks) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(lvl.price, 'f', 2))
              << new QStandardItem(QString::number(lvl.quantity, 'f', 8));
        asksModel->appendRow(items);
    }
    ui->tableView->setModel(asksModel);
    ui->tableView->resizeColumnsToContents();


    // --- BIDS TABLE ---
    QStandardItemModel *bidsModel = new QStandardItemModel(this);
    bidsModel->setColumnCount(2);
    bidsModel->setHorizontalHeaderLabels({"Bid Price", "Bid Qty"});

    for (const auto &lvl : bids) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(lvl.price, 'f', 2))
              << new QStandardItem(QString::number(lvl.quantity, 'f', 8));
        bidsModel->appendRow(items);
    }
    ui->tableView_2->setModel(bidsModel);
    ui->tableView_2->resizeColumnsToContents();
}
