/*


 * Author: Rohit Kumar
 * Contact: rohit312003@gmail.com
 * Date: 15-11-2025

*/

#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include<QAbstractButton>
namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:

    void on_buttonBox_accepted();

private:
    void loadSavedCredentials();  // Load credentials on startup

    Ui::Login *ui;

};

#endif // LOGIN_H
