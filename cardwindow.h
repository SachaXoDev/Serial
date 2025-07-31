#ifndef CARDWINDOW_H
#define CARDWINDOW_H

#include <QDialog>
#include <QList>
#include <QPair>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class CardWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CardWindow(const QList<QPair<QString, QString>>& cards, QWidget *parent = nullptr);

private slots:
    void showNextCard();
    void showPrevCard();
    void flipCard();
    void exitPushCard();
    void animateCardChange();

private:
    void updateCard();
    void toggleDirection();

    QLabel *m_cardLabel;
    QList<QPair<QString, QString>> m_cards;
    int m_currentIndex;
    bool m_showWord;
    bool m_forwardDirection;
};

#endif // CARDWINDOW_H
