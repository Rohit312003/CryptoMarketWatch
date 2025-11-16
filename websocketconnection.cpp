#include "websocketconnection.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonParseError>
#include"mainwindow.h"
#include"globals.h"
#include"configmanager.h"

WebSocketConnection::WebSocketConnection(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url)
{
    qRegisterMetaType<CryptoCV::OkxTicker>();

    connect(&m_socket, &QWebSocket::connected, this, &WebSocketConnection::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketConnection::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketConnection::onTextMessageReceived);
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocketConnection::onSocketError);

    connect(this,SIGNAL(tickerReceived(CryptoCV::OkxTicker)), MAIN_WWINDOW_PTR->marketWatchDockWindowPtr->model,SLOT(onBrodcastRcv(CryptoCV::OkxTicker)));
    connect(this,SIGNAL(orderBookReceived(QJsonObject)), MAIN_WWINDOW_PTR->marketWatchDockWindowPtr,SLOT(onTableDoubleClickedResponse(QJsonObject)));


    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_reconnectAttempts < m_maxReconnectAttempts) {
            qDebug() << "Attempting to reconnect..." << m_reconnectAttempts + 1;
            m_reconnectAttempts++;
            connectToServer();
        } else {
            emit errorOccured("Reconnect attempts exhausted");
            m_reconnectTimer.stop();
        }
    });

    m_pingTimer.setInterval(25000);
    connect(&m_pingTimer, &QTimer::timeout, this, &WebSocketConnection::onPingTimeout);
}

// --- WebSocket Methods ---

void WebSocketConnection::connectToServer()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) return;
    qDebug() << "Connecting to" << m_url.toString();
    m_socket.open(m_url);
}

void WebSocketConnection::subscribeTicker(const QString &instId)
{
    subscribeTickers(QStringList() << instId);
}

void WebSocketConnection::subscribeTickers(const QStringList &instIds)
{
    if (instIds.isEmpty()) return;

    QJsonArray args;
    for (const QString &id : instIds) {
        if (id.isEmpty() || !id.contains('-')) {
            qWarning() << "Invalid instId format:" << id << ". Should be like 'BTC-USDT'.";
            continue;
        }

        // 1. Fetch the initial snapshot via REST API.
        // This is a non-blocking call. The reply will be handled asynchronously in onRestReply.
        qDebug() << "Fetching initial snapshot for" << id;
        fetchTickerSnapshot(id);

        // 2. Prepare the WebSocket subscription message argument.
        QJsonObject a;
        a["channel"] = "tickers";
        a["instId"] = id;
        args.append(a);
    }

    if(args.isEmpty()) return;

    // 3. Send the full subscription message over the WebSocket.
    // This is also non-blocking and allows live updates to start arriving.
    QJsonObject obj;
    obj["op"] = "subscribe";
    obj["args"] = args;
    sendJson(obj);
}

void WebSocketConnection::sendJson(const QJsonObject &obj)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        qWarning() << "WebSocket not connected - cannot send message.";
        return;
    }
    QJsonDocument doc(obj);
    m_socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void WebSocketConnection::onConnected()
{
    qDebug() << "WebSocket connected successfully.";
    m_reconnectAttempts = 0;
    m_reconnectTimer.stop();
    m_pingTimer.start();
    emit connected();

    // ---- Resubscribe all symbols from INI ----
    QSettings settings(ConfigManager::instance().configPath(), QSettings::IniFormat);
    settings.beginGroup("MarketWatch/CryptoRows");
    QStringList symbolList = settings.value("symbols").toStringList();
    settings.endGroup();

    if (!symbolList.isEmpty()) {
        subscribeTickers(symbolList); // your subscribe method: takes QStringList of symbols
        qDebug() << "Resubscribed tokens after reconnect:" << symbolList;
    }
}


void WebSocketConnection::onDisconnected()
{
    qDebug() << "WebSocket disconnected.";
    m_pingTimer.stop();
    emit disconnected();
    if (!m_reconnectTimer.isActive()) {
        m_reconnectTimer.start();
    }
}

void WebSocketConnection::onPingTimeout()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Sending ping to server...";
        m_socket.sendTextMessage("ping");
    }
}

void WebSocketConnection::onTextMessageReceived(const QString &msg)
{
    if (msg == "pong") {
        qDebug() << "Received pong from server.";
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << err.errorString() << "Original message:" << msg;
        return;
    }
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();

    if (obj.contains("event")) {
        QString event = obj.value("event").toString();
        if (event == "error") {
            QString errMsg = obj.value("msg").toString();
            qWarning() << "Subscription error:" << errMsg;
            emit errorOccured("Subscription Error: " + errMsg);
        } else if (event == "subscribe") {
            qDebug() << "Successfully subscribed to channel:" << obj.value("arg").toObject().value("channel").toString();
        }
        return;
    }
    handleIncomingJson(obj);
}

void WebSocketConnection::handleIncomingJson(const QJsonObject &obj)
{
    QJsonObject arg = obj.value("arg").toObject();
    if (arg.isEmpty() || arg.value("channel").toString() != "tickers") return;
    if (!obj.contains("data") || !obj["data"].isArray()) return;
    QJsonArray data = obj["data"].toArray();

    for (const QJsonValue &v : data) {
        if (!v.isObject()) continue;
        QJsonObject rec = v.toObject();
        CryptoCV::OkxTicker t;
        t.instId  = rec.value("instId").toString();
        t.last    = rec.value("last").toString().toDouble();
        t.bid     = rec.value("bidPx").toString().toDouble();
        t.ask     = rec.value("askPx").toString().toDouble();
        t.askQty  = rec.value("askSz").toString().toDouble();
        t.bidQty  = rec.value("bidSz").toString().toDouble();
        emit tickerReceived(t);
    }
}

void WebSocketConnection::onSocketError(QAbstractSocket::SocketError)
{
    QString err = m_socket.errorString();
    qWarning() << "WebSocket error:" << err;
    emit errorOccured(err);
}

// --- REST API Methods (Snapshot) ---

void WebSocketConnection::fetchTickerSnapshot(const QString &instId)
{
    QString url = QString("https://www.okx.com/api/v5/market/ticker?instId=%1").arg(instId);
    QNetworkRequest req{QUrl(url)};
    QNetworkReply* reply = m_networkManager.get(req);
    connect(reply, &QNetworkReply::finished, this, &WebSocketConnection::onRestReply);
}

void WebSocketConnection::makeApiRequest(CryptoCV::ApiRequestType type, const QString &symbol, int limit)
{
    QString url;
    switch (type) {
    case CryptoCV::ApiRequestType::TickerSnapshot:
        url = QString("https://www.okx.com/api/v5/market/ticker?instId=%1").arg(symbol);
        break;
    case CryptoCV::ApiRequestType::OrderBookSnapshot:
        url = QString("https://www.okx.com/api/v5/market/books?instId=%1&sz=%2").arg(symbol).arg(limit);
        break;
    case CryptoCV::ApiRequestType::RecentTrades:
        url = QString("https://www.okx.com/api/v5/market/trades?instId=%1&limit=%2").arg(symbol).arg(limit);
        break;
        // Add more cases as needed
    }
    QNetworkRequest req{QUrl(url)};
    QNetworkReply* reply = m_networkManager.get(req);
    m_replyTypeMap[reply] = type;
    connect(reply, &QNetworkReply::finished, this, &WebSocketConnection::onGenericApiReply);
}


void WebSocketConnection::onRestReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray resp = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(resp, &err);
        if (err.error != QJsonParseError::NoError) {
            emit errorOccured("REST JSON parse error: " + err.errorString());
            reply->deleteLater();
            return;
        }
        QJsonObject obj = doc.object();
        if (obj.contains("data") && obj["data"].isArray()) {
            QJsonArray arr = obj["data"].toArray();
            if (!arr.isEmpty()) {
                QJsonObject rec = arr.first().toObject();
                CryptoCV::OkxTicker t;
                t.instId  = rec.value("instId").toString();
                t.last    = rec.value("last").toString().toDouble();
                t.bid     = rec.value("bidPx").toString().toDouble();
                t.ask     = rec.value("askPx").toString().toDouble();
                t.askQty  = rec.value("askSz").toString().toDouble();
                t.bidQty  = rec.value("bidSz").toString().toDouble();
                emit tickerReceived(t);
            }
        }
    } else {
        emit errorOccured("REST error: " + reply->errorString());
    }
    reply->deleteLater();
}

void WebSocketConnection::onGenericApiReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    CryptoCV::ApiRequestType type = m_replyTypeMap.value(reply, CryptoCV::ApiRequestType::TickerSnapshot); // default fallback

    QByteArray resp = reply->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(resp, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorOccured("REST JSON parse error");
        reply->deleteLater();
        m_replyTypeMap.remove(reply);
        return;
    }

    QJsonObject obj = doc.object();

    switch (type) {
    case CryptoCV::ApiRequestType::TickerSnapshot:
        // process ticker

        emit tickerSnapshotReceived(obj);
        break;
    case CryptoCV::ApiRequestType::OrderBookSnapshot:

        emit orderBookReceived(obj);
        break;
    case CryptoCV::ApiRequestType::RecentTrades:
        // process trades
        emit recentTradesReceived(obj);
        break;
        // Add more cases as needed
    }
    reply->deleteLater();
    m_replyTypeMap.remove(reply);
}

