#include "globals.h"
// Global pointer to main window instance (for cross-object communication)
mainWindow* MAIN_WWINDOW_PTR = nullptr;

// Global pointer to WebSocketConnection instance (for market/API calls)
WebSocketConnection* WEB_SOCKET_CONNECTION = nullptr;
