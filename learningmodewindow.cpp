#include "learningmodewindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <algorithm>
#include <random>

LearningModeWindow::LearningModeWindow(int moduleId, const QList<QPair<QString, QString>>& cards,
                                       MainWindow *mainWindowParent, QWidget *parent)
    : QDialog(parent), m_moduleId(moduleId), m_cards(cards), m_originalCards(cards),
    m_mainWindow(mainWindowParent), m_currentIndex(0), m_correctAnswers(0), m_answerChecked(false),
    m_showAnswer(false), m_forwardDirection(true), m_statsLabel(new QLabel(this)),
    m_progressBar(new QProgressBar(this))
{
    setWindowTitle("Обучение карточкам");
    setMinimumSize(400, 300);

    this->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "}"

        "QLabel {"
        "   font-size: 28px;"
        "   color: white;"
        "   margin-bottom: 10px;"
        "}"

        "QLineEdit {"
        "   font-size: 18px;"
        "   padding: 8px;"
        "   border: 2px solid rgba(255,255,255,0.4);"
        "   border-radius: 10px;"
        "   background: rgba(255,255,255,0.15);"
        "   color: white;"
        "}"

        "QLineEdit:focus {"
        "   border-color: rgba(255,255,255,0.8);"
        "}"

        "QPushButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"

        "QPushButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "   border-color: rgba(255,255,255,0.5);"
        "}"

        "QPushButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "   border-color: rgba(255,255,255,0.7);"
        "}"

        "QPushButton:disabled {"
        "   color: rgba(255,255,255,0.4);"
        "   border-color: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#checkButton {"
        "   border-color: rgba(255,255,255,0.5);"
        "}"

        "QProgressBar {"
        "   border: 2px solid rgba(255,255,255,0.4);"
        "   border-radius: 8px;"
        "   color: white;"
        "   font-weight: bold;"
        "   text-align: center;"
        "   background: transparent;"
        "}"

        "QProgressBar::chunk {"
        "   background: qlineargradient(stop:0 #8a2be2, stop:1 #9370db);"
        "   border-radius: 5px;"
        "}"

        "#resultLabel {"
        "   font-size: 20px;"  // Увеличим размер шрифта
        "   font-weight: bold;"
        "   padding: 6px;"
        "   border-radius: 10px;"
        "   margin: 10px 0;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "}"

        "QPushButton#starButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: none;"
        "   font-size: 24px;"
        "   padding: 0;"
        "   min-width: 32px;"
        "   max-width: 32px;"
        "   min-height: 32px;"
        "   max-height: 32px;"
        "}"

        "QPushButton#starButton:hover {"
        "   color: white;"
        "}"

        "QPushButton#navButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"

        "QPushButton#navButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#navButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#navButton:disabled {"
        "   color: rgba(255,255,255,0.4);"
        "   border-color: rgba(255,255,255,0.2);"
        "}"

        );

    m_progressBar->move(310, 120);
    m_progressBar->resize(150, 20);

    QVBoxLayout *layout = new QVBoxLayout(this);


    QHBoxLayout *topButtonsLayout = new QHBoxLayout();
    buttonsLayout = new QHBoxLayout();

    wordLabel = new QLabel(this);
    wordLabel->setAlignment(Qt::AlignCenter);
    wordLabel->setFont(QFont("Arial", 30));
    layout->addWidget(wordLabel);

    answerLineEdit = new QLineEdit(this);
    answerLineEdit->setPlaceholderText("Введите ответ...");
    answerLineEdit->setClearButtonEnabled(true);
    layout->addWidget(answerLineEdit);

    statsLabel = new QLabel(this);
    statsLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(statsLabel);

    resultLabel = new QLabel(this);
    resultLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(resultLabel);

    QPushButton *exitPush = new QPushButton("Выход", this);
    exitPush->setGeometry(370, 10, 80, 40);
    exitPush->setStyleSheet("padding: 8px;");
    connect(exitPush, &QPushButton::clicked, this, &LearningModeWindow::exitPushCard);

    prevButton = new QPushButton("Назад", this);
    prevButton->setObjectName("navButton");
    randomButton = new QPushButton("Перемешать", this);
    randomButton->setObjectName("navButton");
    nextButton = new QPushButton("Вперед", this);
    nextButton->setObjectName("navButton");

    checkButton = new QPushButton("Проверить", this);
    checkButton->setDisabled(true);
    connect(answerLineEdit, &QLineEdit::textChanged, this, &LearningModeWindow::enableCheckButtonIfAnswerProvided);

    showAnswerButton = new QPushButton("Показать ответ", this);
    showAnswerButton->setDisabled(true);

    resetOrderButton = new QPushButton("Исходный порядок", this);

    starButton = new QPushButton("☆", this);
    starButton->setObjectName("starButton");
    starButton->setGeometry(320, 210, 32, 32);
    starButton->setProperty("isFavorite", false);

    if (m_mainWindow && !m_cards.isEmpty()) {
        QString word = m_cards[m_currentIndex].first;
        QString translation = m_cards[m_currentIndex].second;

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM favorites WHERE user_id = :user_id AND module_id = :module_id "
                           "AND word = :word AND translation = :translation");
        checkQuery.bindValue(":user_id", m_mainWindow->getUserId());
        checkQuery.bindValue(":module_id", m_moduleId);
        checkQuery.bindValue(":word", word);
        checkQuery.bindValue(":translation", translation);

        if (checkQuery.exec() && checkQuery.next()) {
            starButton->setText("★");
            starButton->setProperty("isFavorite", true);
        }
    }

    connect(starButton, &QPushButton::clicked, this, &LearningModeWindow::likeCards);

    topButtonsLayout->addWidget(randomButton);
    topButtonsLayout->addWidget(resetOrderButton);
    topButtonsLayout->addStretch();
    layout->addLayout(topButtonsLayout);

    buttonsLayout->addWidget(prevButton);
    buttonsLayout->addWidget(checkButton);
    buttonsLayout->addWidget(showAnswerButton);
    buttonsLayout->addWidget(nextButton);
    layout->addLayout(buttonsLayout);

    m_answerResults.resize(m_cards.size());
    std::fill(m_answerResults.begin(), m_answerResults.end(), false);

    connectSignals();
    updateCard();
    updateStats();
    updateNavigationButtons();
}

void LearningModeWindow::connectSignals()
{
    connect(prevButton, &QPushButton::clicked, this, &LearningModeWindow::handleNavigation);
    connect(nextButton, &QPushButton::clicked, this, &LearningModeWindow::handleNavigation);
    connect(checkButton, &QPushButton::clicked, this, &LearningModeWindow::checkAnswer);
    connect(showAnswerButton, &QPushButton::clicked, this, &LearningModeWindow::toggleAnswer);
    connect(answerLineEdit, &QLineEdit::returnPressed, this, &LearningModeWindow::checkAnswer);
    connect(randomButton, &QPushButton::clicked, this, &LearningModeWindow::randomCards);
    connect(resetOrderButton, &QPushButton::clicked, this, &LearningModeWindow::resetCardOrder);
}

void LearningModeWindow::enableCheckButtonIfAnswerProvided()
{
    bool hasAnswer = !answerLineEdit->text().trimmed().isEmpty();
    checkButton->setEnabled(hasAnswer);
}

void LearningModeWindow::handleNavigation()
{
    QObject* senderObj = sender();
    if(senderObj == prevButton) {
        if(m_forwardDirection) {
            m_currentIndex = (m_currentIndex == 0) ? m_cards.size() - 1 : m_currentIndex - 1;
        } else if(m_currentIndex > 0) {
            m_currentIndex--;
        }
    }
    else if(senderObj == nextButton) {
        if(m_forwardDirection) {
            m_currentIndex = (m_currentIndex == m_cards.size() - 1) ? 0 : m_currentIndex + 1;
        } else if(m_currentIndex < m_cards.size() - 1) {
            m_currentIndex++;
        }
    }

    m_answerChecked = false;
    m_showAnswer = false;
    showAnswerButton->setText("Показать ответ");
    updateCard();
    updateNavigationButtons();
}

void LearningModeWindow::updateCard()
{
    if(m_cards.isEmpty())
        return;

    resultLabel->setStyleSheet("");
    resultLabel->clear();

    wordLabel->setText(m_cards[m_currentIndex].first);
    answerLineEdit->clear();

    if (!m_answerResults[m_currentIndex]) {
        m_answerChecked = false;
        m_showAnswer = false;
    }

    checkButton->setText("Проверить");
    checkButton->setEnabled(false);
    showAnswerButton->setText("Показать ответ");
    showAnswerButton->setEnabled(m_answerResults[m_currentIndex]);
    resultLabel->clear();

    if (m_answerResults[m_currentIndex] && m_showAnswer) {
        answerLineEdit->setText(m_cards[m_currentIndex].second);
        showAnswerButton->setText("Скрыть ответ");
    }

    updateStarButton();

    updateTextColor();

    setWindowTitle(QString("Карточка %1/%2").arg(m_currentIndex+1).arg(m_cards.size()));
}

void LearningModeWindow::toggleAnswer()
{
    if (!m_answerResults[m_currentIndex]) {
        return;
    }

    m_showAnswer = !m_showAnswer;
    if (m_showAnswer) {
        answerLineEdit->setText(m_cards[m_currentIndex].second);
        showAnswerButton->setText("Скрыть ответ");
    } else {
        answerLineEdit->clear();
        showAnswerButton->setText("Показать ответ");
    }
}

void LearningModeWindow::checkAnswer()
{
    if(m_answerChecked) {
        if(m_currentIndex < m_cards.size()-1) {
            m_currentIndex++;
            m_answerChecked = false;
            m_showAnswer = false;
            updateCard();
            updateNavigationButtons();
        } else {
            checkCompletion();
        }
        return;
    }

    if(m_answerChecked) {
        if(m_currentIndex < m_cards.size()-1) {
            m_currentIndex++;
            m_answerChecked = false;
            m_showAnswer = false;
            updateCard();
            updateNavigationButtons();
        }
        return;
    }

    QString userAnswer = answerLineEdit->text().trimmed();
    QString correctAnswer = m_cards[m_currentIndex].second.trimmed();

    bool isCorrect = userAnswer.compare(correctAnswer, Qt::CaseInsensitive) == 0;
    m_answerResults[m_currentIndex] = isCorrect;

    if(isCorrect) {
        resultLabel->setText("✓ Верно!");
        resultLabel->setStyleSheet(
            "color: #4CAF50;"
            "font-size: 20px;"
            "font-weight: bold;"
            "background-color: rgba(0, 0, 0, 0.4);"
            "border: 2px solid #4CAF50;"
            "border-radius: 10px;"
            "padding: 3px;"
            );
        m_correctAnswers++;
    }
    else {
        resultLabel->setText("✗ Неверно! Правильный ответ: " + correctAnswer);
        resultLabel->setStyleSheet(
            "color: #FF5252;"
            "font-size: 20px;"
            "font-weight: bold;"
            "background-color: rgba(0, 0, 0, 0.4);"
            "border: 2px solid #FF5252;"
            "border-radius: 10px;"
            "padding: 3px;"
            );
    }

    m_answerChecked = true;
    showAnswerButton->setEnabled(true);

    updateStats();
    updateTextColor();

    if (isCorrect) {
        checkCompletion();
    }
}


void LearningModeWindow::updateStats()
{
    m_progressBar->setMaximum(m_cards.size());
    m_progressBar->setValue(m_correctAnswers);
    QString formatStr = QString("%1/%2").arg(m_correctAnswers).arg(m_cards.size());
    m_progressBar->setFormat(formatStr);
}

void LearningModeWindow::updateNavigationButtons()
{
    bool hasMultipleCards = (m_cards.size() > 1);

    if(m_forwardDirection) {
        prevButton->setEnabled(hasMultipleCards);
        nextButton->setEnabled(hasMultipleCards);
    } else {
        prevButton->setEnabled(hasMultipleCards && (m_currentIndex > 0));
        nextButton->setEnabled(hasMultipleCards && (m_currentIndex < m_cards.size() - 1));
    }

    randomButton->setEnabled(hasMultipleCards);
    resetOrderButton->setEnabled(hasMultipleCards && (m_cards != m_originalCards));
}

void LearningModeWindow::randomCards()
{
    if (m_cards.size() < 2) {
        QMessageBox::information(this, "Недостаточно карточек", "Для перемешивания нужно как минимум 2 карточки.");
        return;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);

    m_currentIndex = 0;
    m_correctAnswers = 0;
    m_answerChecked = false;
    m_showAnswer = false;
    showAnswerButton->setText("Показать ответ");

    updateCard();
    updateStats();
    updateNavigationButtons();
    answerLineEdit->clear();
    resultLabel->clear();
}

void LearningModeWindow::resetCardOrder()
{
    if (m_cards.size() < 2) {
        QMessageBox::information(this, "Недостаточно карточек", "Только одна карточка - порядок не изменен.");
        return;
    }

    m_cards = m_originalCards;
    m_currentIndex = 0;
    m_correctAnswers = 0;
    m_answerChecked = false;
    m_showAnswer = false;
    showAnswerButton->setText("Показать ответ");

    updateCard();
    updateStats();
    updateNavigationButtons();
    answerLineEdit->clear();
    resultLabel->clear();
}



void LearningModeWindow::likeCards()
{
    if (m_cards.isEmpty() || m_currentIndex < 0 || m_currentIndex >= m_cards.size()) {
        return;
    }

    if (!m_mainWindow) {
        return;
    }

    QString word = m_cards[m_currentIndex].first;
    QString translation = m_cards[m_currentIndex].second;
    bool isFavorite = starButton->property("isFavorite").toBool();

    if (isFavorite) {
        if (m_mainWindow->removeFromFavorites(m_mainWindow->getUserId(), word, translation)) {
            starButton->setText("☆");
            starButton->setProperty("isFavorite", false);
        }
    } else {
        if (m_mainWindow->addToFavorites(m_mainWindow->getUserId(), m_moduleId, word, translation)) {
            starButton->setText("★");
            starButton->setProperty("isFavorite", true);
        }
    }
}

void LearningModeWindow::updateStarButton()
{
    if (!m_mainWindow) return;

    QString word = m_cards[m_currentIndex].first;
    QString translation = m_cards[m_currentIndex].second;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM favorites WHERE user_id = :user_id AND module_id = :module_id "
                       "AND word = :word AND translation = :translation");
    checkQuery.bindValue(":user_id", m_mainWindow->getUserId());
    checkQuery.bindValue(":module_id", m_moduleId);
    checkQuery.bindValue(":word", word);
    checkQuery.bindValue(":translation", translation);

    if (checkQuery.exec() && checkQuery.next()) {
        starButton->setText("★");
        starButton->setProperty("isFavorite", true);
    } else {
        starButton->setText("☆");
        starButton->setProperty("isFavorite", false);
    }
}

void LearningModeWindow::updateTextColor()
{
    if (m_answerResults[m_currentIndex]) {
        wordLabel->setStyleSheet("color: green;");
    } else if (m_answerChecked && !m_answerResults[m_currentIndex]) {
        wordLabel->setStyleSheet("color: red;");
    } else {
        wordLabel->setStyleSheet("color: black;");
    }
}

void LearningModeWindow::checkCompletion()
{
    bool allCorrect = std::all_of(m_answerResults.begin(), m_answerResults.end(), [](bool val) { return val; });

    if (allCorrect && !m_cards.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Поздравляем!");
        msgBox.setText("🎉 Молодец! Все карточки отвечены правильно!");

        QPushButton *continueButton = msgBox.addButton("Продолжить обучение", QMessageBox::AcceptRole);
        QPushButton *exitButton = msgBox.addButton("Выйти", QMessageBox::RejectRole);

        msgBox.setDefaultButton(continueButton);
        msgBox.exec();

        if (msgBox.clickedButton() == exitButton) {
            this->accept();
        }
    }
}

void LearningModeWindow::exitPushCard()
{
    this->accept();
}
