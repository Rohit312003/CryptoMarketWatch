#include <QDialogButtonBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QDebug>
#include <QMessageBox>
#include "login.h"
#include "ui_login.h"
#include "websocketconnection.h"
#include "configmanager.h"
#include "globals.h"

extern WebSocketConnection* WEB_SOCKET_CONNECTION;

Login::Login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    // Set password field to echo mode (hidden characters)
    ui->lineEdit_2->setEchoMode(QLineEdit::Password);

    // Load saved credentials if they exist
    loadSavedCredentials();

    // Connect accepted and rejected signals from the button box
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Login::on_buttonBox_accepted,Qt::UniqueConnection);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Login::~Login()
{
    delete ui;
}

void Login::loadSavedCredentials()
{
    QString username = ConfigManager::instance().getUsername();
    // QString password = ConfigManager::instance().getPassword();


    // Load username
    if (!username.isEmpty()) {
        ui->lineEdit->setText(username);
        qDebug() << "Loaded username:" << username;
    }

    // // Load password (displayed as masked characters)
    // if (!password.isEmpty()) {
    //     ui->lineEdit_2->setText(password);
    //     qDebug() << "Loaded password (masked)";
    // }

}

void Login::on_buttonBox_accepted()
{
    // Get credentials from UI
    QString username = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    // Validate input
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Username or password cannot be empty.");
        reject();
        return;

    }else if (username == ConfigManager::instance().getUsername() && password == ConfigManager::instance().getPassword()) {
        // Save credentials and connection mode
        ConfigManager::instance().setUsername(username);
        ConfigManager::instance().setPassword(password);
        ConfigManager::instance().save();

        qDebug() << "Credentials saved: username =" << username;

        // Create WebSocket connection
        WEB_SOCKET_CONNECTION = new WebSocketConnection();
        qDebug() << "Connecting via WebSocket...";
        WEB_SOCKET_CONNECTION->connectToServer();


        accept();// this goes in mainwindow funtion connectLogin()
         return;
    } else {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
       reject(); // this goes in mainwindow funtion connectLogin()
         return;
    }
}
