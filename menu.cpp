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

Menu::Menu(QWidget *parent, MainWindow *mainWindow) :
    QDialog(parent),
    ui(new Ui::Menu),
    m_isDialogOpen(false),
    m_mainWindow(mainWindow) // Инициализация m_mainWindow
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
        "QScrollArea { border: none; }"
        "QWidget#scrollAreaWidgetContents { background: transparent; }"
        "QLabel { font-size: 16px; }"
        "QTableWidget { font-size: 16px; }"
        "QLineEdit {"
        "   border: 1px solid #ccc;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "}"
        );

    ui->emptyLabel->setVisible(false);
    ui->emptyLabel->setStyleSheet("color: gray;");

    ui->addModuleButton->installEventFilter(this);

    modulesLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    modulesLayout->setAlignment(Qt::AlignTop);

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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

    QWidget *moduleWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(moduleWidget);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QPushButton *nameButton = new QPushButton(moduleName, moduleWidget);
    topLayout->addWidget(nameButton);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *studyButton = new QPushButton("Карточки", moduleWidget);
    QPushButton *educationButton = new QPushButton("Проверка знаний", moduleWidget);
    QPushButton *manageCardsButton = new QPushButton("Управление карточками", moduleWidget);
    QPushButton *deleteButton = new QPushButton("Удалить модуль", moduleWidget);

    deleteButton->setProperty("moduleId", moduleId);
    educationButton->setProperty("moduleId", moduleId);
    studyButton->setProperty("moduleId", moduleId);
    manageCardsButton->setProperty("moduleId", moduleId);

    QVBoxLayout *leftColumn = new QVBoxLayout();
    QVBoxLayout *rightColumn = new QVBoxLayout();

    leftColumn->addWidget(studyButton);
    leftColumn->addWidget(educationButton);

    rightColumn->addWidget(manageCardsButton);
    rightColumn->addWidget(deleteButton);

    leftColumn->setSpacing(5);
    rightColumn->setSpacing(5);

    studyButton->setVisible(false);
    educationButton->setVisible(false);
    manageCardsButton->setVisible(false);
    deleteButton->setVisible(false);

    bottomLayout->addLayout(leftColumn);
    bottomLayout->addLayout(rightColumn);
    bottomLayout->setContentsMargins(2, 0, 0, 0);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    moduleWidget->setLayout(mainLayout);

    modulesLayout->addWidget(moduleWidget);

    connect(nameButton, &QPushButton::clicked, this, [studyButton, educationButton, manageCardsButton, deleteButton]() {
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

    QSqlQuery query;
    query.prepare("DELETE FROM modules WHERE id = :id AND user_id = :userId");
    query.bindValue(":id", moduleId);
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить модуль: " + query.lastError().text());
        return;
    }

    QWidget *moduleWidget = button->parentWidget();
    modulesLayout->removeWidget(moduleWidget);
    delete moduleWidget;

    if (modulesLayout->count() == 0) {
        ui->emptyLabel->setVisible(true);
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

    dialog.setAttribute(Qt::WA_KeyCompression, false);

    if (dialog.exec() == QDialog::Accepted) {
        QString moduleName = dialog.textValue().trimmed();

        if (!moduleName.isEmpty()) {
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT id FROM modules WHERE user_id = :userId AND module_name = :name");
            checkQuery.bindValue(":userId", userId);
            checkQuery.bindValue(":name", moduleName);

            if (checkQuery.exec() && !checkQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO modules (user_id, module_name) VALUES (:userId, :name)");
                insertQuery.bindValue(":userId", userId);
                insertQuery.bindValue(":name", moduleName);

                if (insertQuery.exec()) {
                    addModuleToUi(insertQuery.lastInsertId().toInt(), moduleName);
                }
            }
        }
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

        LearningModeWindow *learnWindow = new LearningModeWindow(cards, this);
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

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    QTableWidget *table = new QTableWidget(dialog);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Слово", "Перевод"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->verticalHeader()->setDefaultSectionSize(40);
    table->setColumnWidth(0, 200);
    table->setColumnWidth(1, 200);
    table->horizontalHeader()->setStretchLastSection(true);

    QFont font = table->font();
    font.setPointSize(14);
    table->setFont(font);

    table->setRowCount(cards.size());
    for (int i = 0; i < cards.size(); ++i) {
        QTableWidgetItem *wordItem = new QTableWidgetItem(cards[i].first);
        wordItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, wordItem);

        QTableWidgetItem *translationItem = new QTableWidgetItem(cards[i].second);
        translationItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 1, translationItem);

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        widget->setLayout(layout);

        table->setCellWidget(i, 2, widget);
    }

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

    connect(addButton, &QPushButton::clicked, this, [this, moduleId, table, mainWindow]() {
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
        if(cancelBtn) cancelBtn->setText("Закрыть");

        form->addRow(buttons);

        connect(buttons, &QDialogButtonBox::accepted, addDialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, addDialog, &QDialog::reject);

        if (addDialog->exec() == QDialog::Accepted) {
            QString word = wordEdit->text().trimmed();
            QString translation = translationEdit->text().trimmed();

            if (!word.isEmpty() && !translation.isEmpty()) {
                bool success = mainWindow->addCardToModule(moduleId, word, translation);
                if (success) {
                    int row = table->rowCount();
                    table->insertRow(row);
                    table->setItem(row, 0, new QTableWidgetItem(word));
                    table->item(row, 0)->setTextAlignment(Qt::AlignCenter);
                    table->setItem(row, 1, new QTableWidgetItem(translation));
                    table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
                } else {
                    QMessageBox::warning(this, "Ошибка", "Не удалось добавить карточку");
                }
            }
        }
        addDialog->deleteLater();
    });

    connect(editButton, &QPushButton::clicked, this, [this, table, moduleId, mainWindow]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите карточку для редактирования");
            return;
        }

        QString oldWord = table->item(row, 0)->text();
        QString oldTranslation = table->item(row, 1)->text();

        QDialog *editDialog = new QDialog(this);
        editDialog->setWindowTitle("Редактировать карточку");

        QFormLayout *form = new QFormLayout(editDialog);
        QLineEdit *wordEdit = new QLineEdit(oldWord, editDialog);
        QLineEdit *translationEdit = new QLineEdit(oldTranslation, editDialog);

        form->addRow("Слово:", wordEdit);
        form->addRow("Перевод:", translationEdit);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, editDialog);
        form->addRow(buttons);

        connect(buttons, &QDialogButtonBox::accepted, editDialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);

        if (editDialog->exec() == QDialog::Accepted) {
            QString newWord = wordEdit->text().trimmed();
            QString newTranslation = translationEdit->text().trimmed();

            if (!newWord.isEmpty() && !newTranslation.isEmpty()) {
                if (mainWindow->updateCardInModule(moduleId, oldWord, oldTranslation, newWord, newTranslation)) {
                    table->item(row, 0)->setText(newWord);
                    table->item(row, 1)->setText(newTranslation);

                    table->item(row, 0)->setTextAlignment(Qt::AlignCenter);
                    table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
                }
            }
        }
        editDialog->deleteLater();
    });

    connect(deleteButton, &QPushButton::clicked, this, [this, table, moduleId, mainWindow]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите карточку для удаления");
            return;
        }

        QString word = table->item(row, 0)->text();
        QString translation = table->item(row, 1)->text();

        if (QMessageBox::question(this, "Подтверждение",
                                  QString("Удалить карточку '%1 - %2'?").arg(word).arg(translation)) == QMessageBox::Yes) {
            if (mainWindow->deleteCardFromModule(moduleId, word, translation)) {
                table->removeRow(row);
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

}

