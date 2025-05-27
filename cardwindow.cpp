#include "cardwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>

CardWindow::CardWindow(const QList<QPair<QString, QString>>& cards, QWidget *parent)
    : QDialog(parent), m_cards(cards), m_currentIndex(0),
    m_showWord(true), m_forwardDirection(true), m_cardLabel(nullptr)
{
    setWindowTitle("Изучение карточек");
    setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *cardContainer = new QWidget(this);
    cardContainer->setObjectName("cardContainer");
    QVBoxLayout *containerLayout = new QVBoxLayout(cardContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    m_cardLabel = new QLabel(cardContainer);
    m_cardLabel->setAlignment(Qt::AlignCenter);
    m_cardLabel->setObjectName("cardLabel");
    m_cardLabel->setStyleSheet("font-size: 24px; padding: 20px; background: white; border: 1px solid #ccc; border-radius: 10px;");
    m_cardLabel->setFixedHeight(200);

    containerLayout->addWidget(m_cardLabel);
    layout->addWidget(cardContainer, 1);

    QPushButton *flipButton = new QPushButton("Перевернуть", this);
    flipButton->setStyleSheet("padding: 8px; font-size: 16px;");
    connect(flipButton, &QPushButton::clicked, this, &CardWindow::flipCard);

    QHBoxLayout *navLayout = new QHBoxLayout();

    QPushButton *exitPush = new QPushButton("Выход", this);
    exitPush->setGeometry(309, 11, 80, 30);
    exitPush->setStyleSheet("padding: 8px; font-size: 16px;");

    QPushButton *prevButton = new QPushButton("Назад", this);
    prevButton->setStyleSheet("padding: 8px; font-size: 16px;");

    QPushButton *nextButton = new QPushButton("Далее", this);
    nextButton->setStyleSheet("padding: 8px; font-size: 16px;");

    connect(exitPush, &QPushButton::clicked, this, &CardWindow::exitPushCard);
    connect(prevButton, &QPushButton::clicked, this, &CardWindow::showPrevCard);
    connect(nextButton, &QPushButton::clicked, this, &CardWindow::showNextCard);

    navLayout->addWidget(prevButton);
    navLayout->addWidget(nextButton);

    layout->addWidget(flipButton);
    layout->addLayout(navLayout);

    updateCard();
}

void CardWindow::showNextCard()
{
    if (m_forwardDirection) {
        m_currentIndex = (m_currentIndex + 1) % m_cards.size();
    } else {
        m_currentIndex = (m_currentIndex - 1 + m_cards.size()) % m_cards.size();
    }
    m_showWord = true;
    updateCard();
}

void CardWindow::showPrevCard()
{
    if (m_forwardDirection) {
        m_currentIndex = (m_currentIndex - 1 + m_cards.size()) % m_cards.size();
    } else {
        m_currentIndex = (m_currentIndex + 1) % m_cards.size();
    }
    m_showWord = true;
    updateCard();
}

void CardWindow::toggleDirection()
{
    m_forwardDirection = !m_forwardDirection;

    if (m_forwardDirection) {
        m_currentIndex = 0;
    } else {
        m_currentIndex = m_cards.size() - 1;
    }

    m_showWord = true;
    updateCard();

}

void CardWindow::flipCard()
{
    QPropertyAnimation *animation = new QPropertyAnimation(m_cardLabel, "geometry");
    animation->setDuration(500);

    QRect startRect = m_cardLabel->geometry();
    QRect midRect = QRect(startRect.x(), startRect.y() + startRect.height()/2,
                          startRect.width(), 1);
    QRect endRect = startRect;

    animation->setKeyValueAt(0, startRect);
    animation->setKeyValueAt(0.5, midRect);
    animation->setKeyValueAt(1, endRect);

    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        m_showWord = !m_showWord;
        updateCard();
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardWindow::updateCard()
{
    if (m_cards.isEmpty()) return;

    QLabel *cardLabel = findChild<QLabel*>("cardLabel");
    if (m_showWord) {
        cardLabel->setText(m_cards[m_currentIndex].first);
    } else {
        cardLabel->setText(m_cards[m_currentIndex].second);
    }
    setWindowTitle(QString("Карточка %1/%2").arg(m_currentIndex+1).arg(m_cards.size()));
}



void CardWindow::exitPushCard()
{
    this->close();
}

