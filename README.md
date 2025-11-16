# CryptoMW – Crypto Market Watch Desktop App

A Qt 6.x desktop platform for live crypto market data display (OKX API) with a clean UI, modular design, and responsive table view.

---

## Features

- **Live Market Data Table**  
  - Displays real-time, auto-updating market prices for top cryptocurrencies (BTC, ETH, SOL, etc.).
  - Table view supports sorting (double-click column header), filtering (UID/Symbol), and dynamic show/hide of columns (right-click or CTRL+H).
  - Drag-and-drop columns for custom arrangement.
  - "Add Symbol" lets users input and watch new crypto symbols instantly.

- **Order Book Snapshot**  
  - Double-click any table cell (excluding headers) to fetch and display a snapshot of the current order book for that symbol.
  - The order book is not live-updating; each double-click triggers a one-time API call for the latest depth.
  - Provides a focused, on-demand view of market depth and order flow at the moment of request.

- **Robust & Responsive UI**  
  - Color-coding for price changes, handling of loading, empty, error, and reconnect states.

---

**Summary:**  
CryptoMW provides a flexible, professional trader-style interface for live crypto tracking and instant order book insights—customizable for power users and suitable for production-scale GUI evaluation.

---

## Installation

1. Install Qt 6.x (Core, Gui, Widgets, WebSockets modules required)
2. Download or clone the source code
3. Open project in Qt creator
4. During configuration, set the compiler to **MinGW 64-bit**
5. To run in debug mode:
```
Set the build directory to NX_BLOCKS_PROJECT\Debug
```
6. To run in release mode:
``` 
Set the build directory to NX_BLOCKS_PROJECT\Release   
```
7. Build and run the project as usual

---

## Usage

- Launch the app and login with the demo credentials
- View and track symbols in the Market Watch table
- Use the “Add Symbol” input to include new cryptocurrency tickers for live updates
- The table supports sorting/filtering, color-coded updates, and dynamic columns

---

## Architecture Overview

A. Two global pointers are declared in `globals.h`:
   - `mainWindow* MAIN_WINDOW_PTR`: Global pointer to the main window object.
   - `WebSocketConnection* WEBSOCKET_CONNECTION_PTR`: Global pointer for managing WebSocket network operations.

B. Application Flow (Startup Sequence):

1. In `main.cpp`, a dynamic object of `mainWindow` is created and saved to the global pointer `MAIN_WINDOW_PTR`.
2. Immediately after creation, the `connectLogin()` function of `mainWindow` is called.  
   This method initializes and launches the login UI.
3. Inside `mainWindow`, there are dynamic pointers for two key widgets:  
   - `login`: The login dialog class  
   - `marketWatchDockWindow`: The docked widget for displaying the market watch table
   These are initialized as part of `mainWindow` setup.
4. When `connectLogin()` is invoked:
   - The login dialog appears.
   - Upon button acceptance, the dialog verifies username and password against stored credentials.
   - If credentials are correct:
     - A dynamic object of `WebSocketConnection` is created and stored in the global pointer.
     - The login dialog returns `accept()` to `mainWindow`.
5. If login is successful (`accept()` returned):
   - The main window UI is shown.
   - The market watch dock widget, managed by its pointer, is displayed.
   - The WebSocket connection is active, streaming crypto data into the table view.
   - Summary:
     - The application uses two global pointers (mainWindow and WebSocketConnection) for centralized object management across UI and network.
     - Startup is managed through a well-defined sequence: window creation, login verification, initialization of the live market watch view.
   
C. Live Data Updates & Efficient Row Handling (For symbol subscription and Marketdata Flow) :
  - After a symbol is subscribed, the incoming market data from OKX WebSocket is processed in the `handleIncomingJson` function.
  - This function emits the signal `tickerReceived`, which is connected to the slot `onBrodcastRcv` in `MarketWatchModel`.
  - The slot updates the appropriate row in the model with new live data.
  - WebSocket updates are very fast and can sometimes send repeated values for the same symbol.
  - To optimize and avoid unnecessary UI refreshes:
  - The code checks if any value has actually changed (price, quantity, etc.) for the symbol.
  - If no change is detected, the incoming data is discarded.
  - Only actual changes trigger table/UI updates to keep the interface efficient and responsive.
  - Summary:
    - Efficient data handling ensures the market watch table updates only when needed. This avoids redundant redraws and ensures smooth performance, even when the WebSocket sends rapid or duplicate ticks

---
Market data flow:  
   - OKX WebSocket → WebSocketConnection → MarketWatchModel (signals) → QTableView UI
---


