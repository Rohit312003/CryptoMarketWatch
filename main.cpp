/****************************************************************************
 * CryptoMW - Crypto Market Watch Desktop App
 *
 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025
 *
 * Description:
 *   Qt main application entry point.
 *   - Sets up QApplication and internationalization (translator)
 *   - Configures and shows main window (mainWindow)
 *   - Assigns global pointer for cross-module access
 *   - Starts login connection procedure
 *
 * Disclaimer:
 *   This software is provided for evaluation and educational purposes only.
 *   No warranty is expressed or implied. Do not use for production trading
 *   without review and modification according to your needs.
 ****************************************************************************/

#include <QApplication>
#include <QMainWindow>
#include <QTranslator>
#include <QLocale>
#include <QTimer>
#include "globals.h"
#include "mainwindow.h"

// Global pointer declaration so all modules can access mainWindow instance (see globals.h)
extern mainWindow* MAIN_WWINDOW_PTR;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Internationalization: load translation depending on system locale
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CRYPTO_MW_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // Main window setup
    mainWindow *mainWind = new mainWindow();
    // Assign global ptr for cross-object/module access (used by market data, dialogs...)
    MAIN_WWINDOW_PTR = mainWind;

    // Post-initialization: kick-off login connect after 1 second
    QTimer::singleShot(1000, MAIN_WWINDOW_PTR, SLOT(connectLogin()));

    // Enter Qt main event loop
    return a.exec();
}
