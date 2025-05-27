#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registrationwindow.h"
#include "menu.h"
#include <random>  // Для std::random_device и std::mt19937
#include <algorithm> // Для std::shuffle

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setStyleSheet(
        "QMainWindow { background: #f5f5f5; }"
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

    qDebug() << "Database path:" << getDatabasePath();
    connectToDatabase();
    createTables();
    showUsers();
}

MainWindow::~MainWindow()
{
    QSqlDatabase::database().close();
    delete ui;
}

QString MainWindow::getDatabasePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dataPath + "/mydatabase.db";
}

void MainWindow::connectToDatabase()
{
    QString dbPath = getDatabasePath();

    // if(QFile::exists(dbPath)) {
    //     if(!QFile::remove(dbPath)) {
    //         qDebug() << "Не удалось удалить старую базу данных";
    //         return;
    //     }
    //     qDebug() << "Старая база данных удалена";
    // }

    // QDir().mkpath(QFileInfo(dbPath).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    // Настройки для избежания блокировок
    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000;QSQLITE_OPEN_READWRITE|QSQLITE_OPEN_CREATE");

    if (!db.open()) {
        qDebug() << "Ошибка подключения к базе данных:" << db.lastError().text();
    } else {
        qDebug() << "Новая база данных создана:" << dbPath;
        createTables(); // Создаем таблицы
    }
}

void MainWindow::createTables()
{
    QSqlQuery query;

    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT NOT NULL UNIQUE, "
                    "password TEXT NOT NULL)")) {
        qDebug() << "Ошибка. Таблица users не создана";
        qDebug() << "Детали ошибки:" << query.lastError().text();
    } else {
        qDebug() << "Таблица users успешно создана/найдена.";
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS modules ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "user_id INTEGER NOT NULL, "
                    "module_name TEXT NOT NULL, "
                    "UNIQUE(user_id, module_name), "
                    "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE)")) {
        qDebug() << "Ошибка создания таблицы modules:" << query.lastError().text();
    } else {
        qDebug() << "Таблица modules успешно создана/найдена.";
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS cards ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "module_id INTEGER NOT NULL, "
                    "word TEXT NOT NULL, "
                    "translation TEXT NOT NULL, "
                    "FOREIGN KEY(module_id) REFERENCES modules(id) ON DELETE CASCADE)")) {
        qDebug() << "Ошибка создания таблицы cards:" << query.lastError().text();
    } else {
        qDebug() << "Таблица cards успешно создана/найдена.";
    }
}

void MainWindow::insertUser(const QString &username, const QString &password)
{
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Ошибка: пользователь не вставлен.";
        qDebug() << "Детали ошибки:" << query.lastError().text();
        QSqlDatabase::database().rollback();
    } else {
        QSqlDatabase::database().commit();
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
    query.prepare("SELECT id, password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса:" << query.lastError().text();
        QMessageBox::warning(this, "Ошибка", "Ошибка при проверке пользователя!");
        return;
    }

    if (query.next()) {
        QString truePassword = query.value("password").toString();
        int userId = query.value("id").toInt();
        if (password == truePassword) {
            QMessageBox::information(this, "Успех", "Пользователь авторизирован!");

            currentUserId = userId;
            showModulesForCurrentUser();

            this->hide();
            Menu *menuWindow = new Menu(this);
            menuWindow->setUserId(currentUserId);
            menuWindow->setModal(true);

            connect(menuWindow, &Menu::exitPush, this, &MainWindow::show);
            connect(menuWindow, &Menu::finished, menuWindow, &Menu::deleteLater);

            menuWindow->exec();

        } else {
            QMessageBox::warning(this, "Ошибка", "Неправильный пароль!");
        }
    } else {
        QMessageBox::warning(this, "Ошибка", "Пользователь не зарегистрирован!");
    }
}

void MainWindow::showModulesForCurrentUser()
{
    if(currentUserId < 0) {
        qDebug() << "Пользователь не авторизован, показать модули невозможно";
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT module_name FROM modules WHERE user_id = :userId");
    query.bindValue(":userId", currentUserId);

    if (!query.exec()) {
        qDebug() << "Ошибка при загрузке модулей для пользователя:" << query.lastError().text();
        return;
    }

    qDebug() << "Модули пользователя (user_id =" << currentUserId << "):";
    while (query.next()) {
        QString moduleName = query.value("module_name").toString();
        qDebug() << " - " << moduleName;
    }
}

bool MainWindow::moduleExists(int userId, const QString &moduleName)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM modules WHERE user_id = :userId AND module_name = :moduleName");
    query.bindValue(":userId", userId);
    query.bindValue(":moduleName", moduleName);

    if (!query.exec()) {
        qDebug() << "Ошибка при проверке существования модуля:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

void MainWindow::deleteModule(int userId, const QString &moduleName)
{
    if (userId < 0 || moduleName.isEmpty()) {
        qDebug() << "Ошибка: неверные параметры для удаления модуля.";
        return;
    }

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("DELETE FROM modules WHERE user_id = :userId AND module_name = :moduleName");
    query.bindValue(":userId", userId);
    query.bindValue(":moduleName", moduleName);

    if (!query.exec()) {
        qDebug() << "Ошибка при удалении модуля:" << query.lastError().text();
        QSqlDatabase::database().rollback();
    } else {
        QSqlDatabase::database().commit();
        qDebug() << "Модуль" << moduleName << "успешно удалён для пользователя с ID:" << userId;
    }
}


QList<QPair<QString, QString>> MainWindow::getCardsForModule(int moduleId)
{
    QList<QPair<QString, QString>> cards;
    QSqlQuery query;
    query.prepare("SELECT word, translation FROM cards WHERE module_id = :module_id");
    query.bindValue(":module_id", moduleId);

    if (query.exec()) {
        while (query.next()) {
            cards.append(qMakePair(query.value(0).toString(),
                                   query.value(1).toString()));
        }
    }
    return cards;
}

QList<QPair<QString, QString>> MainWindow::getShuffledCardsForModule(int moduleId)
{
    QList<QPair<QString, QString>> cards = getCardsForModule(moduleId);

    // Проверяем, есть ли что перемешивать
    if (cards.size() > 1) {
        // Инициализация генератора случайных чисел
        std::random_device rd;
        std::mt19937 g(rd());

        // Перемешиваем карточки
        std::shuffle(cards.begin(), cards.end(), g);
    }

    return cards;
}

bool MainWindow::addCardToModule(int moduleId, const QString &word, const QString &translation)
{
    QSqlQuery query;
    query.prepare("INSERT INTO cards (module_id, word, translation) VALUES (:moduleId, :word, :translation)");
    query.bindValue(":moduleId", moduleId);
    query.bindValue(":word", word);
    query.bindValue(":translation", translation);

    if (!query.exec()) {
        qDebug() << "Ошибка добавления карточки:" << query.lastError().text();
        return false;
    }
    return true;
}

bool MainWindow::updateCardInModule(int moduleId, const QString &oldWord, const QString &oldTranslation,
                                    const QString &newWord, const QString &newTranslation) {
    QSqlQuery query;
    query.prepare("UPDATE cards SET word = :newWord, translation = :newTranslation "
                  "WHERE module_id = :moduleId AND word = :oldWord AND translation = :oldTranslation");
    query.bindValue(":moduleId", moduleId);
    query.bindValue(":oldWord", oldWord);
    query.bindValue(":oldTranslation", oldTranslation);
    query.bindValue(":newWord", newWord);
    query.bindValue(":newTranslation", newTranslation);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось обновить карточку: " + query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool MainWindow::deleteCardFromModule(int moduleId, const QString &word, const QString &translation) {
    QSqlQuery query;
    query.prepare("DELETE FROM cards WHERE module_id = :moduleId AND word = :word AND translation = :translation");
    query.bindValue(":moduleId", moduleId);
    query.bindValue(":word", word);
    query.bindValue(":translation", translation);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить карточку: " + query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}
