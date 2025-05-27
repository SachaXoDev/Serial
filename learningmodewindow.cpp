#include "learningmodewindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <algorithm>
#include <random>

LearningModeWindow::LearningModeWindow(const QList<QPair<QString, QString>>& cards, QWidget *parent)
    : QDialog(parent), m_cards(cards), m_originalCards(cards), m_currentIndex(0),
    m_correctAnswers(0), m_answerChecked(false), m_showAnswer(false), m_forwardDirection(true)
{
    setWindowTitle("Обучение карточкам");
    setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *topButtonsLayout = new QHBoxLayout();
    buttonsLayout = new QHBoxLayout();

    wordLabel = new QLabel(this);
    wordLabel->setAlignment(Qt::AlignCenter);
    wordLabel->setFont(QFont("Arial", 30));
    layout->addWidget(wordLabel);

    answerLineEdit = new QLineEdit(this);
    answerLineEdit->setPlaceholderText("Введите ответ...");
    layout->addWidget(answerLineEdit);

    statsLabel = new QLabel(this);
    statsLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(statsLabel);

    resultLabel = new QLabel(this);
    resultLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(resultLabel);

    QPushButton *exitPush = new QPushButton("Выход", this);
    exitPush->setGeometry(480, 15, 80, 40);
    exitPush->setStyleSheet("padding: 8px;");
    connect(exitPush, &QPushButton::clicked, this, &LearningModeWindow::exitPushCard);

    prevButton = new QPushButton("Назад", this);
    randomButton = new QPushButton("Перемешать", this);
    nextButton = new QPushButton("Вперед", this);
    checkButton = new QPushButton("Проверить", this);
    showAnswerButton = new QPushButton("Показать ответ", this);
    resetOrderButton = new QPushButton("Исходный порядок", this);

    QString buttonStyle = "QPushButton {padding: 8px 16px; min-width: 100px; border: 1px solid #ccc; border-radius: 5px; background: white;} "
                          "QPushButton:hover {background: #f0f0f0;} "
                          "QPushButton:disabled {background: #e0e0e0; color: #a0a0a0;}";

    prevButton->setStyleSheet(buttonStyle);
    randomButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);
    checkButton->setStyleSheet(buttonStyle);
    showAnswerButton->setStyleSheet(buttonStyle);
    resetOrderButton->setStyleSheet(buttonStyle);

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

void LearningModeWindow::checkAnswer()
{
    if(m_answerChecked) {
        if(m_currentIndex < m_cards.size()-1) {
            m_currentIndex++;
            m_answerChecked = false;
            m_showAnswer = false;
            showAnswerButton->setText("Показать ответ");
            updateCard();
            updateNavigationButtons();
        }
        return;
    }

    QString userAnswer = answerLineEdit->text().trimmed();
    QString correctAnswer = m_cards[m_currentIndex].second;

    if(userAnswer.compare(correctAnswer, Qt::CaseInsensitive) == 0) {
        resultLabel->setText("✓ Верно!");
        resultLabel->setStyleSheet("color: green; font-size: 18px;");
        m_correctAnswers++;
        m_answerResults[m_currentIndex] = true;
    }
    else {
        resultLabel->setText("✗ Неверно! Правильный ответ: " + correctAnswer);
        resultLabel->setStyleSheet("color: red; font-size: 18px;");
        m_answerResults[m_currentIndex] = false;
    }

    m_answerChecked = true;
    checkButton->setText("Далее");
    updateStats();
}

void LearningModeWindow::toggleAnswer()
{
    m_showAnswer = !m_showAnswer;
    if(m_showAnswer) {
        answerLineEdit->setText(m_cards[m_currentIndex].second);
        showAnswerButton->setText("Скрыть ответ");
    }
    else {
        answerLineEdit->clear();
        showAnswerButton->setText("Показать ответ");
    }
}

void LearningModeWindow::updateCard()
{
    if(m_cards.isEmpty()) return;

    wordLabel->setText(m_cards[m_currentIndex].first);
    answerLineEdit->clear();
    resultLabel->clear();
    checkButton->setText("Проверить");

    if(m_answerResults[m_currentIndex]) {
        wordLabel->setStyleSheet("color: green;");
    } else if(!m_answerResults[m_currentIndex] && answerLineEdit->text().isEmpty()) {
        wordLabel->setStyleSheet("color: black;");
    } else {
        wordLabel->setStyleSheet("color: red;");
    }

    setWindowTitle(QString("Карточка %1/%2").arg(m_currentIndex+1).arg(m_cards.size()));
}

void LearningModeWindow::updateStats()
{
    statsLabel->setText(QString("Правильно: %1/%2").arg(m_correctAnswers).arg(m_cards.size()));
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

void LearningModeWindow::exitPushCard()
{
    this->close();
}
