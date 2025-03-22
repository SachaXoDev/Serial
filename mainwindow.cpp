#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registrationwindow.h"
#include "home.h"
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectToDatabase();
    createTables();
    showUsers();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mydatabase.db");

    if (!db.open()) {
        qDebug() << "Ошибка. База данных не подключена.";
        qDebug() << "Детали ошибки:" << db.lastError().text();
    } else {
        qDebug() << "База данных подключена успешно.";
    }
}

void MainWindow::createTables()
{
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY,  "
                    "username TEXT NOT NULL, "
                    "password TEXT NOT NULL)")) {
        qDebug() << "Ошибка. Таблица не создана";
        qDebug() << "Детали ошибки:" << query.lastError().text();
    } else {
        qDebug() << "Таблица успешно создана.";
    }
}

void MainWindow::insertUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Ошибка: пользователь не вставлен.";
        qDebug() << "Детали ошибки:" << query.lastError().text();
    } else {
        qDebug() << "Пользователь вставлен успешно.";
    }
}

void MainWindow::showUsers()
{
    QSqlQuery query("SELECT * FROM users");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString username = query.value("username").toString();
        QString password = query.value("password").toString();
        qDebug() << "ID:" << id << "Username:" << username << "Password:" << password;
    }
}

void MainWindow::on_registationPushButton_clicked()
{
    this->hide();
    RegistrationWindow *window = new RegistrationWindow(this);
    window->setModal(true);

    connect(window, &RegistrationWindow::userRegistered, this, &MainWindow::handleUserRegistration);

    connect(window, &RegistrationWindow::cancelPushButton, this, &MainWindow::show);

    window->exec();
    delete window;
}

void MainWindow::handleUserRegistration(const QString &username, const QString &password)
{
    insertUser(username, password);
    showUsers();

    this->show();
}

void MainWindow::on_ExistPushButton_clicked()
{
    this->close();
}

void MainWindow::on_OkpushButton_clicked()
{
    QString username = ui->loginLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поля не могут быть пустыми!");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса:" << query.lastError().text();
        QMessageBox::warning(this, "Ошибка", "Ошибка при проверке пользователя!");
        return;
    }

    if (query.next()) {
        QString truePassword = query.value("password").toString();
        if (password == truePassword){
            QMessageBox::information(this, "Успех", "Пользователь авторизирован!");
            this->hide();
            Home *win = new Home(this);
            win->setModal(true);
            win->exec();
        } else {
            QMessageBox::warning(this, "Ошибка", "Неправильный пароль!");
        }
    } else {
        QMessageBox::warning(this, "Ошибка", "Пользователь не авторизирован!");
    }

}

