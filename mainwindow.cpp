#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registrationwindow.h"
#include "menu.h"
#include <random>
#include <QGraphicsDropShadowEffect>
#include <QToolButton>
#include <QHBoxLayout>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    this->setStyleSheet(
        "QMainWindow {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "}"

        "QGroupBox {"
        "   background: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(255, 255, 255, 0.3);"
        "   border-radius: 10px;"
        "   margin: 20px;"
        "   padding: 15px;"
        "}"

        "QLabel#titleLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #2d3748;"
        "   padding-bottom: 15px;"
        "}"

        "QLabel {"
        "   font-size: 14px;"
        "   color: #4a5568;"
        "   font-weight: 500;"
        "}"

        "QLineEdit {"
        "   border: 2px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 4px;"
        "   font-size: 14px;"
        "   background: white;"
        "   color: #2d3748;"
        "   selection-background-color: #667eea;"
        "}"

        "QLineEdit:focus {"
        "   border-color: #667eea;"
        "   background: white;"
        "}"

        "QLineEdit::placeholder {"
        "   color: #a0aec0;"
        "   font-style: italic;"
        "}"

        "QPushButton#OkpushButton {"
        "   background: qlineargradient(stop:0 #667eea, stop:1 #764ba2);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 12px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"

        "QPushButton#OkpushButton:hover {"
        "   background: qlineargradient( stop:0 #5a67d8, stop:1 #6b46c1);"
        "}"

        "QPushButton#OkpushButton:pressed {"
        "   background: qlineargradient( stop:0 #4c51bf, stop:1 #553c9a);"
        "}"

        "QPushButton#registrationPushButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#registrationPushButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#registrationPushButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#ExistPushButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#ExistPushButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#ExistPushButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QMessageBox {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "   color: white;"
        "   font-size: 14px;"
        "}"

        "QMessageBox QLabel {"
        "   color: white;"
        "   font-size: 14px;"
        "}"

        "QMessageBox QPushButton {"
        "   background: white;"
        "   color: #764ba2;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   min-width: 80px;"
        "}"

        "QMessageBox QPushButton:hover {"
        "   background: #f7fafc;"
        "}"

        "QMessageBox QPushButton:pressed {"
        "   background: #edf2f7;"
        "}"
        );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 150));
    ui->groupBox->setGraphicsEffect(shadow);

    setupPasswordVisibilityToggle(ui->passwordLineEdit);

    ui->loginLineEdit->setPlaceholderText("Введите логин");
    ui->passwordLineEdit->setPlaceholderText("Введите пароль");

    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);

    QRegion region = QRegion(path.toFillPolygon().toPolygon());
    setMask(region);

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

    QDir().mkpath(QFileInfo(dbPath).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

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

    if (!query.exec("CREATE TABLE IF NOT EXISTS favorites ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "user_id INTEGER NOT NULL, "
                    "module_id INTEGER NOT NULL, "
                    "word TEXT NOT NULL, "
                    "translation TEXT NOT NULL, "
                    "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE, "
                    "FOREIGN KEY(module_id) REFERENCES modules(id) ON DELETE CASCADE)")) {
        qDebug() << "Ошибка создания таблицы favorites:" << query.lastError().text();
    } else {
        qDebug() << "Таблица favorites успешно создана/найдена.";
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

void MainWindow::on_registrationPushButton_clicked()
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

    if (cards.size() > 1) {
        std::random_device rd;
        std::mt19937 g(rd());

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

bool MainWindow::toggleFavoriteCard(int moduleId, const QString &word, const QString &translation, bool favorite)
{
    QSqlDatabase::database().transaction();

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM favorites WHERE user_id = :user_id AND module_id = :module_id "
                       "AND word = :word AND translation = :translation");
    checkQuery.bindValue(":user_id", currentUserId);
    checkQuery.bindValue(":module_id", moduleId);
    checkQuery.bindValue(":word", word);
    checkQuery.bindValue(":translation", translation);

    if (!checkQuery.exec()) {
        qDebug() << "Ошибка при проверке избранного:" << checkQuery.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    bool success = false;
    if (favorite) {
        if (!checkQuery.next()) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO favorites (user_id, module_id, word, translation) "
                                "VALUES (:user_id, :module_id, :word, :translation)");
            insertQuery.bindValue(":user_id", currentUserId);
            insertQuery.bindValue(":module_id", moduleId);
            insertQuery.bindValue(":word", word);
            insertQuery.bindValue(":translation", translation);

            success = insertQuery.exec();
            if (!success) {
                qDebug() << "Ошибка при добавлении в избранное:" << insertQuery.lastError().text();
            }
        }
    } else {
        if (checkQuery.next()) {
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM favorites WHERE user_id = :user_id AND module_id = :module_id "
                                "AND word = :word AND translation = :translation");
            deleteQuery.bindValue(":user_id", currentUserId);
            deleteQuery.bindValue(":module_id", moduleId);
            deleteQuery.bindValue(":word", word);
            deleteQuery.bindValue(":translation", translation);

            success = deleteQuery.exec();
            if (!success) {
                qDebug() << "Ошибка при удалении из избранного:" << deleteQuery.lastError().text();
            }
        }
    }

    if (success) {
        QSqlDatabase::database().commit();
    } else {
        QSqlDatabase::database().rollback();
    }
    return success;
}

QList<QPair<QPair<QString, QString>, int>> MainWindow::getFavoriteCards(int userId)
{
    QList<QPair<QPair<QString, QString>, int>> favorites;
    QSqlQuery query;
    query.prepare("SELECT word, translation, module_id FROM favorites WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            favorites.append(qMakePair(
                qMakePair(query.value(0).toString(), query.value(1).toString()),
                query.value(2).toInt()
                ));
        }
    } else {
        qDebug() << "Ошибка при получении избранных карточек:" << query.lastError().text();
    }
    return favorites;
}

bool MainWindow::addToFavorites(int userId, int moduleId, const QString& word, const QString& translation)
{
    QSqlQuery query;
    query.prepare("INSERT INTO favorites (user_id, module_id, word, translation) "
                  "VALUES (:user_id, :module_id, :word, :translation)");
    query.bindValue(":user_id", userId);
    query.bindValue(":module_id", moduleId);
    query.bindValue(":word", word);
    query.bindValue(":translation", translation);

    return query.exec();
}

bool MainWindow::removeFromFavorites(int userId, const QString& word, const QString& translation)
{
    QSqlQuery query;
    query.prepare("DELETE FROM favorites WHERE user_id = :user_id "
                  "AND word = :word AND translation = :translation");
    query.bindValue(":user_id", userId);
    query.bindValue(":word", word);
    query.bindValue(":translation", translation);

    return query.exec();
}

bool MainWindow::isCardFavorite(int userId, int moduleId, const QString &word, const QString &translation) {
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM favorites "
        "WHERE user_id = :user_id AND module_id = :module_id "
        "AND word = :word AND translation = :translation"
        );
    query.bindValue(":user_id", userId);
    query.bindValue(":module_id", moduleId);
    query.bindValue(":word", word);
    query.bindValue(":translation", translation);

    if (!query.exec()) {
        qDebug() << "Ошибка при проверке избранного:" << query.lastError().text();
        return false;
    }

    return query.next() && query.value(0).toInt() > 0;
}

void MainWindow::setupPasswordVisibilityToggle(QLineEdit *passwordLineEdit)
{
    QToolButton *toggleButton = new QToolButton(passwordLineEdit);
    toggleButton->setText("👀");
    toggleButton->setCheckable(true);
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    toggleButton->setToolTip("Показать/скрыть пароль");

    QHBoxLayout *layout = new QHBoxLayout(passwordLineEdit);
    layout->addWidget(toggleButton, 0, Qt::AlignRight);
    layout->setContentsMargins(0, 0, 5, 0);
    passwordLineEdit->setLayout(layout);

    connect(toggleButton, &QToolButton::toggled, this, [passwordLineEdit](bool checked) {
        passwordLineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });
}
