#include "registrationwindow.h"
#include "ui_registrationwindow.h"

RegistrationWindow::RegistrationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegistrationWindow)
{
    ui->setupUi(this);

    this->setStyleSheet(
        "QDialog { background: #f5f5f5; }"
        "QPushButton {"
        "   background: white;"
        "   border: 1px solid #ccc;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover { background: #e6e6e6; }"
        "QLineEdit {"
        "   border: 1px solid #ccc;"
        "   border-radius: 4px;"
        "   padding: 4px;"
        "   font-size: 12px;"
        "}"
        );
}

RegistrationWindow::~RegistrationWindow()
{
    delete ui;
}

void RegistrationWindow::on_cancelPushButton_clicked()
{
    emit cancelPushButton();
    this->close();
}

void RegistrationWindow::on_registerPushButton_clicked()
{
    QString username = ui->loginLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString password_2 = ui->passwordLineEdit_2->text();

    if (username.isEmpty() || password.isEmpty() || password_2.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поля не могут быть пустыми!");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса:" << query.lastError().text();
        QMessageBox::warning(this, "Ошибка", "Ошибка при проверке пользователя!");
        return;
    }

    if (query.next()) {
        QMessageBox::warning(this, "Ошибка", "Логин уже существует!");
        return;
    }

    if (password != password_2) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают!");
        return;
    }

    emit userRegistered(username, password);

    QMessageBox::information(this, "Успех", "Пользователь зарегистрирован!");
    this->close();
}
