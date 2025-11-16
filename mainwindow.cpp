/*


 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025



*/

#include "mainwindow.h"
#include <QApplication>
#include <QTimer>

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("CRYPTO_MW");
    loginPage = new Login(this);
    marketWatchDockWindowPtr = new marketWatchDockWindow(this);
    addDockWidget(Qt::LeftDockWidgetArea, marketWatchDockWindowPtr);
}

void mainWindow::connectLogin()
{
    if (loginPage->exec() == QDialog::Accepted) {
        // Show main window only if login succeeds
        this->showMaximized();

        // Delay loading of rows (e.g., by 500 ms)
        QTimer::singleShot(500, this->marketWatchDockWindowPtr, [this]() {
            this->marketWatchDockWindowPtr->loadCryptoRowsFromIni(); // load crypto from ini with delay
        });
    } else {
        qApp->quit(); // Quit if login is cancelled or fails
    }
}
