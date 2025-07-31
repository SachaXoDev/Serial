#include "cardwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

CardWindow::CardWindow(const QList<QPair<QString, QString>>& cards, QWidget *parent)
    : QDialog(parent), m_cards(cards), m_currentIndex(0),
    m_showWord(true), m_forwardDirection(true), m_cardLabel(nullptr)
{
    setWindowTitle("Изучение карточек");
    setMinimumSize(500, 400);

    this->setStyleSheet(
        "QDialog {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "}"

        "QWidget#cardContainer {"
        "   background: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid rgba(255, 255, 255, 0.3);"
        "   border-radius: 15px;"
        "   padding: 20px;"
        "}"

        "QLabel#cardLabel {"
        "   font-size: 28px;"
        "   padding: 30px;"
        "   background: white;"
        "   border-radius: 10px;"
        "   border: 2px solid #e2e8f0;"
        "   min-height: 250px;"
        "   color: #2d3748;"
        "}"

        "QPushButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 12px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"

        "QPushButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#navButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-size: 16px;"
        "}"

        "QPushButton#navButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#navButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"

        "QPushButton#exitButton {"
        "   background: transparent;"
        "   color: white;"
        "   border: 2px solid rgba(255,255,255,0.3);"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-size: 16px;"
        "}"

        "QPushButton#exitButton:hover {"
        "   background: rgba(255,255,255,0.1);"
        "}"

        "QPushButton#exitButton:pressed {"
        "   background: rgba(255,255,255,0.2);"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QWidget *cardContainer = new QWidget(this);
    cardContainer->setObjectName("cardContainer");

    QVBoxLayout *containerLayout = new QVBoxLayout(cardContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    m_cardLabel = new QLabel(cardContainer);
    m_cardLabel->setAlignment(Qt::AlignCenter);
    m_cardLabel->setObjectName("cardLabel");
    m_cardLabel->setWordWrap(true);

    containerLayout->addWidget(m_cardLabel);
    mainLayout->addWidget(cardContainer, 1);

    QPushButton *flipButton = new QPushButton("↻ Перевернуть", this);
    connect(flipButton, &QPushButton::clicked, this, &CardWindow::flipCard);

    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(15);

    QPushButton *prevButton = new QPushButton("Назад", this);
    QPushButton *nextButton = new QPushButton("Далее", this);
    QPushButton *exitButton = new QPushButton("✕ Выход", this);
    exitButton->setObjectName("exitButton");

    connect(exitButton, &QPushButton::clicked, this, &CardWindow::exitPushCard);
    connect(prevButton, &QPushButton::clicked, this, &CardWindow::showPrevCard);
    connect(nextButton, &QPushButton::clicked, this, &CardWindow::showNextCard);

    navLayout->addWidget(prevButton);
    navLayout->addStretch(1);
    navLayout->addWidget(exitButton);
    navLayout->addStretch(1);
    navLayout->addWidget(nextButton);

    mainLayout->addWidget(flipButton);
    mainLayout->addLayout(navLayout);

    updateCard();
}

void CardWindow::flipCard()
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(m_cardLabel);
    m_cardLabel->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(300);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);

    connect(animation, &QPropertyAnimation::finished, this, [this, effect]() {
        m_showWord = !m_showWord;
        updateCard();

        QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardWindow::showNextCard()
{
    if (m_forwardDirection) {
        m_currentIndex = (m_currentIndex + 1) % m_cards.size();
    } else {
        m_currentIndex = (m_currentIndex - 1 + m_cards.size()) % m_cards.size();
    }
    m_showWord = true;
    animateCardChange();
}

void CardWindow::showPrevCard()
{
    if (m_forwardDirection) {
        m_currentIndex = (m_currentIndex - 1 + m_cards.size()) % m_cards.size();
    } else {
        m_currentIndex = (m_currentIndex + 1) % m_cards.size();
    }
    m_showWord = true;
    animateCardChange();
}

void CardWindow::animateCardChange()
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(m_cardLabel);
    m_cardLabel->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(200);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);

    connect(animation, &QPropertyAnimation::finished, this, [this, effect]() {
        updateCard();

        QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
        fadeIn->setDuration(200);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardWindow::updateCard()
{
    if (m_cards.isEmpty()) return;

    if (m_showWord) {
        m_cardLabel->setText(QString("<div style='font-size:32px; color:#333;'>%1</div>").arg(m_cards[m_currentIndex].first));
    } else {
        m_cardLabel->setText(QString("<div style='font-size:28px; color:#666;'>%1</div>").arg(m_cards[m_currentIndex].second));
    }
    setWindowTitle(QString("Карточка %1/%2").arg(m_currentIndex+1).arg(m_cards.size()));
}

void CardWindow::exitPushCard()
{
    this->close();
}

