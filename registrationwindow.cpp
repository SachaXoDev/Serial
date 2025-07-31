#include "registrationwindow.h"
#include "ui_registrationwindow.h"
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>

RegistrationWindow::RegistrationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegistrationWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    this->setStyleSheet(
        "QDialog {"
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

        "QPushButton#registerPushButton {"
        "   background: qlineargradient(stop:0 #667eea, stop:1 #764ba2);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"

        "QPushButton#registerPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #5a67d8, stop:1 #6b46c1);"
        "}"

        "QPushButton#registerPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4c51bf, stop:1 #553c9a);"
        "}"

        "QPushButton#cancelPushButton {"
        "   background: white;"
        "   color: #4a5568;"
        "   border: 2px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "}"

        "QPushButton#cancelPushButton:hover {"
        "   border-color: #cbd5e0;"
        "   background: #f7fafc;"
        "}"

        "QPushButton#cancelPushButton:pressed {"
        "   background: #edf2f7;"
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

    ui->loginLineEdit->setPlaceholderText("Придумайте логин");
    ui->passwordLineEdit->setPlaceholderText("Придумайте пароль");
    ui->passwordLineEdit_2->setPlaceholderText("Повторите пароль");

    setupPasswordVisibilityToggle(ui->passwordLineEdit);
    setupPasswordVisibilityToggle(ui->passwordLineEdit_2);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->passwordLineEdit_2->setEchoMode(QLineEdit::Password);

    QPainterPath path;
    path.addRoundedRect(rect(), 15, 15);

    QRegion region = QRegion(path.toFillPolygon().toPolygon());
    setMask(region);
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
        QMessageBox::warning(this, "Ошибка", "Поля не могут быть пустыми!",
                             QMessageBox::Ok);
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Ошибка при проверке пользователя!",
                             QMessageBox::Ok);
        return;
    }

    if (query.next()) {
        QMessageBox::warning(this, "Ошибка", "Логин уже существует!",
                             QMessageBox::Ok);
        return;
    }

    if (password != password_2) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают!",
                             QMessageBox::Ok);
        return;
    }

    emit userRegistered(username, password);

    QMessageBox::information(this, "Успех", "Пользователь зарегистрирован!",
                             QMessageBox::Ok);
    this->close();
}

void RegistrationWindow::setupPasswordVisibilityToggle(QLineEdit *passwordLineEdit)
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
