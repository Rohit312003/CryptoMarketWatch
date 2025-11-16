#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"login.h"
#include"marketwatchdockwindow.h"
class mainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit mainWindow(QWidget *parent = nullptr);
    Login* loginPage=nullptr;//this ptr used to redirect to login page
/*

this ptr link mainwindow with marketWatch so that by using global mainWindow ptr we can manupulate the marketWatchDockWindow class object

*/
    marketWatchDockWindow* marketWatchDockWindowPtr=nullptr;
public slots:
/*
     this is for login page call

*/
    void connectLogin();

};

#endif // MAINWINDOW_H
