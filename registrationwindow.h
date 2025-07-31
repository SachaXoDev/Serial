#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QDialog>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <QPainterPath>
#include <QLineEdit>

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();

signals:
    void userRegistered(const QString &username, const QString &password);
    void cancelPushButton();

private slots:
    void on_cancelPushButton_clicked();
    void on_registerPushButton_clicked();

private:
    Ui::RegistrationWindow *ui;
    void setupPasswordVisibilityToggle(QLineEdit *passwordLineEdit);
};

#endif // REGISTRATIONWINDOW_H
