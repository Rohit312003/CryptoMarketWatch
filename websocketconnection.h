/*
 * WebSocketConnection.h
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * @class WebSocketConnection
 * @brief Main class for handling connection between this application and OKX:
 *        - Real-time market data via public WebSocket
 *        - REST API requests for ticker snapshots, order book, and trades
 *        Routes received data to Qt models, UI, and logs. Handles reconnection logic.
 */

#pragma once

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "protocol.h" // For CryptoCV::OkxTicker and ApiRequestType enums

class WebSocketConnection : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor: Sets up Okx WebSocket with default endpoint.
     * @param url WebSocket endpoint for subscription (default = OKX public)
     * @param parent Optional parent QObject
     */
    explicit WebSocketConnection(const QUrl &url = QUrl("wss://ws.okx.com:8443/ws/v5/public"),
                                 QObject *parent = nullptr);

    // WebSocket methods

    /**
     * Connects/reconnects to the OKX WebSocket server.
     */
    void connectToServer();

    /**
     * Subscribes for live ticker updates for one instrument.
     * @param instId Instrument symbol ("BTC-USDT", etc.)
     */
    void subscribeTicker(const QString &instId);

    /**
     * Subscribes for live ticker updates for multiple instruments,
     * and fetches initial REST snapshot for each.
     * @param instIds List of instrument symbols
     */
    void subscribeTickers(const QStringList &instIds);

    // REST API methods

    /**
     * Fetches snapshot data (current stats) for a symbol with REST API.
     * @param instId Instrument symbol
     */
    void fetchTickerSnapshot(const QString &instId);

    /**
     * Makes a REST API request (ticker, orderbook, trades, etc.) and routes reply using a switch-case-based handler.
     * @param type Type of API request (enum)
     * @param symbol Instrument symbol ("BTC-USDT", etc.)
     * @param limit Optional: Depth (orderbook) or count (trades)
     */
    void makeApiRequest(CryptoCV::ApiRequestType type, const QString &symbol, int limit);

signals:
    // Connection status
    void connected();
    void disconnected();
    void errorOccured(const QString &err);

    // Crypto ticker updates (from WebSocket or REST)
    void tickerReceived(CryptoCV::OkxTicker ticker);
    void tickerSnapshotReceived(QJsonObject obj);

    // Order book and trades (REST responses)
    void orderBookReceived(QJsonObject obj);
    void recentTradesReceived(QJsonObject obj);

private slots:
    // WebSocket event handling
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &msg);
    void onSocketError(QAbstractSocket::SocketError error);
    void onPingTimeout();

    // REST/Network reply handling
    void onRestReply();
    void onGenericApiReply();

private:
    // Helper to send JSON payloads via WebSocket
    void sendJson(const QJsonObject &obj);

    // Parses incoming WebSocket JSON messages
    void handleIncomingJson(const QJsonObject &obj);

    // Members
    QWebSocket m_socket;                  // For streaming/live updates
    QNetworkAccessManager m_networkManager;   // For REST API requests
    QUrl m_url;
    QTimer m_reconnectTimer;              // Auto-reconnect timer
    QTimer m_pingTimer;                   // Send periodic pings to keep alive

    QMap<QNetworkReply*, CryptoCV::ApiRequestType> m_replyTypeMap; // For dispatching generic REST API replies

    int m_reconnectAttempts = 0;
    const int m_maxReconnectAttempts = 10;
};
