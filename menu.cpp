#include "menu.h"
#include "ui_menu.h"
#include "mainwindow.h"
#include "cardwindow.h"
#include "learningmodewindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidget>
#include <QFormLayout>
#include <QHeaderView>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

Menu::Menu(QWidget *parent, MainWindow *mainWindow) :
    QDialog(parent),
    ui(new Ui::Menu),
    m_isDialogOpen(false),
    m_mainWindow(mainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    this->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "}"

        "QLabel#titleLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: white;"
        "   padding: 10px;"
        "}"

        "QPushButton#exitPush {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#exitPush:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#exitPush:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#favoriteButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#favoriteButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#favoriteButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#addModuleButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"

        "QPushButton#addModuleButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#addModuleButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QLabel#emptyLabel {"
        "   color: rgba(255,255,255,0.7);"
        "   font-size: 16px;"
        "   font-style: italic;"
        "}"
        );

    ui->scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background: transparent;"
        "   border: none;"
        "   padding: 5px;"
        "}"

        "QScrollArea > QWidget > QWidget {"
        "   background: transparent;"
        "}"

        "QScrollBar:vertical {"
        "   border: none;"
        "   background: rgba(255, 255, 255, 0.2);"
        "   width: 8px;"
        "   margin: 0px;"
        "}"

        "QScrollBar::handle:vertical {"
        "   background: rgba(255, 255, 255, 0.5);"
        "   min-height: 30px;"
        "   border-radius: 4px;"
        "}"

        "QScrollBar::handle:vertical:hover {"
        "   background: rgba(255, 255, 255, 0.7);"
        "}"

        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "   background: none;"
        "}"

        "QScrollBar:horizontal {"
        "   height: 0px;"
        "}"
        );

    QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect();
    windowShadow->setBlurRadius(20);
    windowShadow->setXOffset(0);
    windowShadow->setYOffset(5);
    windowShadow->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(windowShadow);

    QGraphicsDropShadowEffect *scrollShadow = new QGraphicsDropShadowEffect();
    scrollShadow->setBlurRadius(15);
    scrollShadow->setXOffset(0);
    scrollShadow->setYOffset(2);
    scrollShadow->setColor(QColor(0, 0, 0, 80));
    ui->scrollArea->setGraphicsEffect(scrollShadow);

    ui->emptyLabel->setVisible(false);
    modulesLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    modulesLayout->setAlignment(Qt::AlignTop);
    modulesLayout->setSpacing(10);
    modulesLayout->setContentsMargins(5, 5, 5, 5);

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPainterPath path;
    path.addRoundedRect(rect(), 15, 15);
    QRegion region = QRegion(path.toFillPolygon().toPolygon());
    setMask(region);

    setupUi();
}

Menu::~Menu()
{
    delete ui;
}

void Menu::setupUi()
{
    QLayoutItem* child;
    while ((child = modulesLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    loadModulesFromDb();
    ui->emptyLabel->setVisible(modulesLayout->count() == 0);
}

void Menu::setUserId(int id)
{
    userId = id;
    setupUi();
}

void Menu::loadModulesFromDb()
{
    if (userId < 0) return;

    QSqlQuery query;
    query.prepare("SELECT id, module_name FROM modules WHERE user_id = :userId");
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка загрузки модулей:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int moduleId = query.value("id").toInt();
        QString moduleName = query.value("module_name").toString();
        addModuleToUi(moduleId, moduleName);
    }
}

void Menu::addModuleToUi(int moduleId, const QString &moduleName)
{
    ui->emptyLabel->setVisible(false);

    QWidget *moduleWidget = new QWidget();
    moduleWidget->setProperty("moduleWidget", true);
    moduleWidget->setStyleSheet(
        "QWidget[moduleWidget=true] {"
        "   background: rgba(255, 255, 255, 0.9);"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"

        "QWidget[moduleWidget=true]:hover {"
        "   background: rgba(255, 255, 255, 0.95);"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(moduleWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QPushButton *nameButton = new QPushButton(moduleName, moduleWidget);
    nameButton->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #2d3748;"
        "   border: none;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   text-align: center;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   color: #667eea;"
        "}"
        );


    topLayout->addStretch();
    topLayout->addWidget(nameButton);
    topLayout->addStretch();

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);

    QString actionButtonStyle =
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "   font-size: 13px;"
        "   min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #5a67d8, stop:1 #6b46c1);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4c51bf, stop:1 #553c9a);"
        "}";

    QPushButton *studyButton = new QPushButton("Карточки", moduleWidget);
    studyButton->setStyleSheet(actionButtonStyle);

    QPushButton *educationButton = new QPushButton("Проверка", moduleWidget);
    educationButton->setStyleSheet(actionButtonStyle);

    QPushButton *manageCardsButton = new QPushButton("Управление", moduleWidget);
    manageCardsButton->setStyleSheet(actionButtonStyle);

    QPushButton *deleteButton = new QPushButton("Удалить", moduleWidget);
    deleteButton->setStyleSheet(actionButtonStyle);

    deleteButton->setProperty("moduleId", moduleId);
    educationButton->setProperty("moduleId", moduleId);
    studyButton->setProperty("moduleId", moduleId);
    manageCardsButton->setProperty("moduleId", moduleId);

    studyButton->setVisible(false);
    educationButton->setVisible(false);
    manageCardsButton->setVisible(false);
    deleteButton->setVisible(false);

    bottomLayout->addWidget(studyButton);
    bottomLayout->addWidget(educationButton);
    bottomLayout->addWidget(manageCardsButton);
    bottomLayout->addWidget(deleteButton);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    moduleWidget->setLayout(mainLayout);

    modulesLayout->addWidget(moduleWidget);

    connect(nameButton, &QPushButton::clicked, this, [=]() {
        bool visible = !studyButton->isVisible();
        studyButton->setVisible(visible);
        educationButton->setVisible(visible);
        manageCardsButton->setVisible(visible);
        deleteButton->setVisible(visible);
    });

    connect(deleteButton, &QPushButton::clicked, this, &Menu::deleteModule);

    connect(studyButton, &QPushButton::clicked, this, [this, moduleId]() {
        MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
        if (mainWindow) {
            auto cards = mainWindow->getCardsForModule(moduleId);
            if (cards.isEmpty()) {
                QMessageBox::information(this, "Карточки", "В этом модуле нет карточек");
                return;
            }

            CardWindow *cardWindow = new CardWindow(cards, this);
            cardWindow->setAttribute(Qt::WA_DeleteOnClose);
            connect(cardWindow, &CardWindow::finished, this, [this]() {
                this->show();
            });
            this->hide();
            cardWindow->exec();
        }
    });

    connect(manageCardsButton, &QPushButton::clicked, this, [this, moduleId, moduleName]() {
        showCardsManagementDialog(moduleId, moduleName);
    });

    connect(educationButton, &QPushButton::clicked, this, [this, moduleId]() {
        startLearningMode(moduleId);
    });
}

void Menu::deleteModule()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int moduleId = button->property("moduleId").toInt();


    QSqlDatabase::database().transaction();

    try {
        QSqlQuery query;

        query.prepare("DELETE FROM favorites WHERE module_id = :moduleId AND user_id = :userId");
        query.bindValue(":moduleId", moduleId);
        query.bindValue(":userId", userId);
        if (!query.exec()) {
            throw std::runtime_error("Не удалось удалить связанные избранные карточки: " +
                                     query.lastError().text().toStdString());
        }

        query.prepare("DELETE FROM cards WHERE module_id = :moduleId");
        query.bindValue(":moduleId", moduleId);
        if (!query.exec()) {
            throw std::runtime_error("Не удалось удалить связанные карточки: " +
                                     query.lastError().text().toStdString());
        }

        query.prepare("DELETE FROM modules WHERE id = :id AND user_id = :userId");
        query.bindValue(":id", moduleId);
        query.bindValue(":userId", userId);
        if (!query.exec()) {
            throw std::runtime_error("Не удалось удалить модуль: " +
                                     query.lastError().text().toStdString());
        }

        if (!QSqlDatabase::database().commit()) {
            throw std::runtime_error("Не удалось завершить транзакцию");
        }

        QWidget *moduleWidget = button->parentWidget();
        if (moduleWidget) {
            modulesLayout->removeWidget(moduleWidget);

            moduleWidget->hide();

            moduleWidget->setAttribute(Qt::WA_DeleteOnClose);
            moduleWidget->deleteLater();
        }

        ui->emptyLabel->setVisible(modulesLayout->count() == 0);

        modulesLayout->update();
        ui->scrollArea->viewport()->update();
        this->update();
        qApp->processEvents();

    } catch (const std::exception& e) {
        QSqlDatabase::database().rollback();
        QMessageBox::warning(this, "Ошибка", e.what());
    }
}

void Menu::on_exitPush_clicked()
{
    emit exitPush();
    this->close();
}

void Menu::on_addModuleButton_clicked()
{
    if (m_isDialogOpen) return;
    m_isDialogOpen = true;

    ui->addModuleButton->clearFocus();

    QInputDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setWindowTitle("Добавить модуль");
    dialog.setLabelText("Введите название модуля:");
    dialog.setTextValue("");
    dialog.setMinimumWidth(300);

    dialog.setOkButtonText("Добавить");
    dialog.setCancelButtonText("Отмена");
    dialog.setStyleSheet(
        "QPushButton {"
        "   background: white;"
        "   color: #764ba2;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "   background: #f7fafc;"
        "}"
        "QPushButton:pressed {"
        "   background: #edf2f7;"
        "}"
        "QPushButton:focus {"
        "   outline: none;"
        "}"
        );

    dialog.setAttribute(Qt::WA_KeyCompression, false);

    QLineEdit *lineEdit = dialog.findChild<QLineEdit*>();
    if (lineEdit) {
        lineEdit->setMaxLength(50);
    }

    while (true) {
        if (dialog.exec() != QDialog::Accepted) {
            break;
        }

        QString moduleName = dialog.textValue().trimmed();

        if (moduleName.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название модуля не может быть пустым!");
            continue;
        }

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM modules WHERE user_id = :userId AND LOWER(module_name) = LOWER(:name)");
        checkQuery.bindValue(":userId", userId);
        checkQuery.bindValue(":name", moduleName);

        if (!checkQuery.exec()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось проверить существование модуля: " + checkQuery.lastError().text());
            continue;
        }

        checkQuery.next();
        int count = checkQuery.value(0).toInt();

        if (count > 0) {
            QMessageBox::warning(this, "Ошибка", "Модуль с таким названием уже существует!");
            continue;
        }

        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO modules (user_id, module_name) VALUES (:userId, :name)");
        insertQuery.bindValue(":userId", userId);
        insertQuery.bindValue(":name", moduleName);

        if (insertQuery.exec()) {
            addModuleToUi(insertQuery.lastInsertId().toInt(), moduleName);
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось создать модуль: " + insertQuery.lastError().text());
        }

        break;
    }

    m_isDialogOpen = false;
    QApplication::processEvents();
}

bool Menu::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->addModuleButton && event->type() == QEvent::MouseButtonPress) {
        static bool processing = false;
        if (processing) return true;
        processing = true;
        on_addModuleButton_clicked();
        processing = false;
        return true;
    }
    return QDialog::eventFilter(watched, event);
}

void Menu::startLearningMode(int moduleId)
{
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
    if (mainWindow) {
        auto cards = mainWindow->getShuffledCardsForModule(moduleId);
        if (cards.isEmpty()) {
            QMessageBox::information(this, "Карточки", "В этом модуле нет карточек");
            return;
        }

        LearningModeWindow *learnWindow = new LearningModeWindow(moduleId, cards, mainWindow);
        learnWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(learnWindow, &LearningModeWindow::finished, this, [this]() {
            this->show();
        });
        this->hide();
        learnWindow->exec();
    }
}


void Menu::showCardsManagementDialog(int moduleId, const QString &moduleName)
{
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
    if (!mainWindow) return;

    QWidget *previousWindow = this->isWindow() ? this : this->window();
    previousWindow->hide();

    auto cards = mainWindow->getCardsForModule(moduleId);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Управление карточками - %1").arg(moduleName));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMinimumSize(600, 400);

    dialog->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "   padding: 15px;"
        "}"

        "QTableWidget {"
        "   background: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(255, 255, 255, 0.3);"
        "   border-radius: 10px;"
        "   font-size: 14px;"
        "   gridline-color: #e2e8f0;"
        "   selection-background-color: rgba(102, 126, 234, 0.3);"
        "   selection-color: #2d3748;"
        "}"

        "QHeaderView::section {"
        "   background-color: rgba(255, 255, 255, 0.8);"
        "   color: #4a5568;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "   border-radius: 5px;"
        "   margin: 1px;"
        "}"

        "QPushButton {"
        "   background: transparent;;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   min-width: 80px;"
        "}"

        "QPushButton:hover {"
        "   background: rgba(255, 255, 255, 0.1);"
        "}"

        "QPushButton:pressed {"
        "   background: rgba(255, 255, 255, 0.1);"
        "}"

        "QPushButton#closeButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255, 255, 255, 0.3);"
        "}"

        "QPushButton#closeButton:hover {"
        "   background: rgba(255, 255, 255, 0.1);"
        "}"

        "QPushButton#closeButton:pressed {"
        "   background: rgba(255, 255, 255, 0.2);"
        "}"

        "QPushButton#starButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 20px;"
        "   color: black;"
        "}"

        "QPushButton#starButton:hover {"
        "   color: #667eea;"
        "}"
        );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialog);
    shadow->setBlurRadius(20);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 100));
    dialog->setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    QTableWidget *table = new QTableWidget(dialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Слово", "Перевод", "Избранное"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->verticalHeader()->setDefaultSectionSize(40);
    table->setColumnWidth(0, 200);
    table->setColumnWidth(1, 200);
    table->setColumnWidth(2, 100);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionsMovable(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->horizontalHeader()->setSectionsClickable(false);
    QFont font = table->font();
    font.setPointSize(14);
    table->setFont(font);

    auto updateStarButton = [this, mainWindow](QPushButton* starButton, int moduleId, const QString& word, const QString& translation) {
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM favorites WHERE user_id = :user_id AND module_id = :module_id "
                           "AND word = :word AND translation = :translation");
        checkQuery.bindValue(":user_id", userId);
        checkQuery.bindValue(":module_id", moduleId);
        checkQuery.bindValue(":word", word);
        checkQuery.bindValue(":translation", translation);

        if (checkQuery.exec() && checkQuery.next()) {
            starButton->setText("★");
            starButton->setProperty("isFavorite", true);
            starButton->setStyleSheet("color: black;");
        } else {
            starButton->setText("☆");
            starButton->setProperty("isFavorite", false);
            starButton->setStyleSheet("");
        }
    };

    auto populateTable = [&]() {
        table->setRowCount(cards.size());
        for (int i = 0; i < cards.size(); ++i) {
            QTableWidgetItem *wordItem = new QTableWidgetItem(cards[i].first);
            wordItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(i, 0, wordItem);

            QTableWidgetItem *translationItem = new QTableWidgetItem(cards[i].second);
            translationItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(i, 1, translationItem);

            QPushButton *starButton = new QPushButton();
            starButton->setFlat(true);
            starButton->setProperty("row", i);
            starButton->setProperty("moduleId", moduleId);
            starButton->setProperty("word", cards[i].first);
            starButton->setProperty("translation", cards[i].second);

            starButton->setObjectName("starButton");

            updateStarButton(starButton, moduleId, cards[i].first, cards[i].second);

            table->setCellWidget(i, 2, starButton);

            connect(starButton, &QPushButton::clicked, this, [this, mainWindow, starButton, &updateStarButton]() {
                int moduleId = starButton->property("moduleId").toInt();
                QString word = starButton->property("word").toString();
                QString translation = starButton->property("translation").toString();
                bool isFavorite = starButton->property("isFavorite").toBool();

                if (mainWindow->toggleFavoriteCard(moduleId, word, translation, !isFavorite)) {
                    updateStarButton(starButton, moduleId, word, translation);
                }
            });
        }
    };

    populateTable();

    table->setStyleSheet(
        "QTableView {"
        "   selection-background-color: transparent;"
        "   selection-color: black;"
        "}"
        "QTableView::item:selected {"
        "   background: transparent;"
        "}"
        );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Добавить", dialog);
    QPushButton *editButton = new QPushButton("Изменить", dialog);
    QPushButton *deleteButton = new QPushButton("Удалить", dialog);
    QPushButton *closeButton = new QPushButton("Закрыть", dialog);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(closeButton);

    mainLayout->addWidget(table);
    mainLayout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, [this, moduleId, table, mainWindow, &cards, &populateTable]() {
        QDialog *addDialog = new QDialog(this);
        addDialog->setWindowTitle("Добавить карточку");

        QFormLayout *form = new QFormLayout(addDialog);
        QLineEdit *wordEdit = new QLineEdit(addDialog);
        QLineEdit *translationEdit = new QLineEdit(addDialog);

        form->addRow("Слово:", wordEdit);
        form->addRow("Перевод:", translationEdit);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, addDialog);

        QPushButton *okBtn = buttons->button(QDialogButtonBox::Ok);
        if(okBtn) okBtn->setText("Ок");

        QPushButton *cancelBtn = buttons->button(QDialogButtonBox::Cancel);
        if(cancelBtn) cancelBtn->setText("Отмена");

        buttons->setStyleSheet(
            "QPushButton {"
            "   background: white;"
            "   color: #764ba2;"
            "   border: none;"
            "   border-radius: 8px;"
            "   padding: 8px 16px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "   min-width: 80px;"
            "}"

            "QPushButton:hover {"
            "   background: #f7fafc;"
            "}"

            "QPushButton:pressed {"
            "   background: #edf2f7;"
            "}"

            "QPushButton:focus {"
            "   outline: none;"
            "}"
            );

        form->addRow(buttons);

        connect(buttons, &QDialogButtonBox::accepted, addDialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, addDialog, &QDialog::reject);

        if (addDialog->exec() == QDialog::Accepted) {
            QString word = wordEdit->text().trimmed();
            QString translation = translationEdit->text().trimmed();

            if (!word.isEmpty() && !translation.isEmpty()) {
                bool success = mainWindow->addCardToModule(moduleId, word, translation);
                if (success) {
                    cards = mainWindow->getCardsForModule(moduleId);
                    populateTable();
                } else {
                    QMessageBox::warning(this, "Ошибка", "Не удалось добавить карточку");
                }
            }
        }
        addDialog->deleteLater();
    });

    connect(editButton, &QPushButton::clicked, this, [this, table, moduleId, mainWindow, &cards, &populateTable]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите карточку для редактирования");
            return;
        }

        QString oldWord = table->item(row, 0)->text();
        QString oldTranslation = table->item(row, 1)->text();

        bool isFavorite = mainWindow->isCardFavorite(userId, moduleId, oldWord, oldTranslation);

        QDialog *editDialog = new QDialog(this);
        editDialog->setWindowTitle("Редактирование");

        QFormLayout *form = new QFormLayout(editDialog);
        QLineEdit *wordEdit = new QLineEdit(oldWord, editDialog);
        QLineEdit *translationEdit = new QLineEdit(oldTranslation, editDialog);

        form->addRow("Слово:", wordEdit);
        form->addRow("Перевод:", translationEdit);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, editDialog);
        form->addRow(buttons);

        QPushButton *okBtn = buttons->button(QDialogButtonBox::Ok);
        if(okBtn) okBtn->setText("Ок");

        QPushButton *cancelBtn = buttons->button(QDialogButtonBox::Cancel);
        if(cancelBtn) cancelBtn->setText("Отмена");

        buttons->setStyleSheet(
            "QPushButton {"
            "   background: white;"
            "   color: #764ba2;"
            "   border: none;"
            "   border-radius: 8px;"
            "   padding: 8px 16px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "   min-width: 80px;"
            "}"

            "QPushButton:hover {"
            "   background: #f7fafc;"
            "}"

            "QPushButton:pressed {"
            "   background: #edf2f7;"
            "}"

            "QPushButton:focus {"
            "   outline: none;"
            "}"
            );

        connect(buttons, &QDialogButtonBox::accepted, editDialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);

        if (editDialog->exec() == QDialog::Accepted) {
            QString newWord = wordEdit->text().trimmed();
            QString newTranslation = translationEdit->text().trimmed();

            if (!newWord.isEmpty() && !newTranslation.isEmpty()) {
                if (mainWindow->updateCardInModule(moduleId, oldWord, oldTranslation, newWord, newTranslation)) {
                    if (isFavorite) {
                        mainWindow->toggleFavoriteCard(moduleId, oldWord, oldTranslation, false);
                        mainWindow->toggleFavoriteCard(moduleId, newWord, newTranslation, true);
                    }
                    cards = mainWindow->getCardsForModule(moduleId);
                    populateTable();
                }
            }
        }
        editDialog->deleteLater();
    });

    connect(deleteButton, &QPushButton::clicked, this, [this, table, moduleId, mainWindow, &cards, &populateTable]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите карточку для удаления");
            return;
        }

        QString word = table->item(row, 0)->text();
        QString translation = table->item(row, 1)->text();

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Подтверждение");
        msgBox.setText(QString("Удалить карточку '%1 - %2'?").arg(word).arg(translation));
        msgBox.setIcon(QMessageBox::Question);

        QPushButton *yesButton = msgBox.addButton("Ок", QMessageBox::YesRole);
        QPushButton *noButton = msgBox.addButton("Отмена", QMessageBox::NoRole);

        msgBox.exec();

        if (msgBox.clickedButton() == yesButton) {
            if (mainWindow->deleteCardFromModule(moduleId, word, translation)) {
                cards = mainWindow->getCardsForModule(moduleId);
                populateTable();
            }
        }
    });

    connect(closeButton, &QPushButton::clicked, this, [this, dialog]() {
        dialog->accept();
        this->show();
    });

    dialog->exec();
}

void Menu::on_favoriteButton_clicked()
{
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
    if (!mainWindow)
        return;

    auto favorites = mainWindow->getFavoriteCards(userId);
    if (favorites.isEmpty()) {
        QMessageBox::information(this, "Избранное", "У вас нет избранных карточек");
        return;
    }

    QDialog *favoritesDialog = new QDialog(nullptr);
    favoritesDialog->setWindowTitle("Избранные карточки");
    favoritesDialog->setAttribute(Qt::WA_DeleteOnClose);
    favoritesDialog->setMinimumSize(200, 400);
    favoritesDialog->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "   padding: 10px;"
        "}"

        "QTableWidget {"
        "   background: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(255, 255, 255, 0.3);"
        "   border-radius: 10px;"
        "   font-size: 14px;"
        "   gridline-color: #e2e8f0;"
        "   selection-background-color: rgba(102, 126, 234, 0.3);"
        "   selection-color: #2d3748;"
        "}"

        "QHeaderView::section {"
        "   background-color: rgba(255, 255, 255, 0.8);"
        "   color: #4a5568;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "   border-radius: 5px;"
        "   margin: 1px;"
        "}"

        "QPushButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 6px;"
        "   padding: 8px 12px;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   min-width: 80px;"
        "}"

        "QPushButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#closeButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255, 255, 255, 0.3);"
        "}"
        );

    QGraphicsDropShadowEffect *favShadow = new QGraphicsDropShadowEffect();
    favShadow->setBlurRadius(20);
    favShadow->setXOffset(0);
    favShadow->setYOffset(5);
    favShadow->setColor(QColor(0, 0, 0, 100));
    favoritesDialog->setGraphicsEffect(favShadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(favoritesDialog);

    QTableWidget *table = new QTableWidget(favoritesDialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Слово", "Перевод"});
    table->setColumnWidth(0, 112);
    table->setColumnWidth(1, 112);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    table->horizontalHeader()->setSectionsMovable(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->horizontalHeader()->setSectionsClickable(false);

    QList<QPair<QPair<QString, QString>, int>> fullFavorites = mainWindow->getFavoriteCards(userId);
    table->setRowCount(fullFavorites.size());

    for (int i = 0; i < fullFavorites.size(); ++i) {
        QTableWidgetItem *wordItem = new QTableWidgetItem(fullFavorites[i].first.first);
        wordItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, wordItem);

        QTableWidgetItem *translationItem = new QTableWidgetItem(fullFavorites[i].first.second);
        translationItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 1, translationItem);

        QTableWidgetItem *moduleIdItem = new QTableWidgetItem(QString::number(fullFavorites[i].second));
        moduleIdItem->setFlags(moduleIdItem->flags() ^ Qt::ItemIsEditable);
        moduleIdItem->setToolTip("Module ID");
        table->setItem(i, 2, moduleIdItem);
    }

    table->setColumnHidden(2, true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton("Удалить", favoritesDialog);
    QPushButton *closeButton = new QPushButton("Закрыть", favoritesDialog);

    deleteButton->setEnabled(false);

    connect(table, &QTableWidget::itemSelectionChanged, this, [deleteButton, table]() {
        deleteButton->setEnabled(table->currentRow() >= 0);
    });

    connect(deleteButton, &QPushButton::clicked, this, [this, mainWindow, table, favoritesDialog]() {
        int row = table->currentRow();
        if (row < 0)
            return;

        QString word = table->item(row, 0)->text();
        QString translation = table->item(row, 1)->text();
        int moduleId = table->item(row, 2)->text().toInt();

        if (QMessageBox::question(favoritesDialog, "Подтверждение",
                                  QString("Удалить '%1 - %2' из избранного?").arg(word).arg(translation))
            == QMessageBox::Yes) {
            if (mainWindow->toggleFavoriteCard(moduleId, word, translation, false)) {
                table->removeRow(row);
                if (table->rowCount() == 0) {
                    favoritesDialog->accept();
                    QMessageBox::information(this, "Избранное", "Все карточки удалены из избранного");
                }
            } else {
                QMessageBox::warning(favoritesDialog, "Ошибка", "Не удалось удалить из избранного");
            }
        }
    });

    connect(closeButton, &QPushButton::clicked, favoritesDialog, &QDialog::accept);

    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(closeButton);
    buttonLayout->setAlignment(Qt::AlignRight);

    mainLayout->addWidget(table);
    mainLayout->addLayout(buttonLayout);

    connect(favoritesDialog, &QDialog::finished, this, [this]() {
        this->show();
    });

    this->hide();
    favoritesDialog->exec();
}
