#ifndef LEARNINGMODEWINDOW_H
#define LEARNINGMODEWINDOW_H

#include <QDialog>
#include <QList>
#include <QPair>

class QLabel;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

class LearningModeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LearningModeWindow(const QList<QPair<QString, QString>>& cards, QWidget *parent = nullptr);

private slots:
    void handleNavigation();
    void checkAnswer();
    void toggleAnswer();
    void exitPushCard();
    void resetCardOrder();
    void randomCards();

private:
    void updateCard();
    void updateStats();
    void updateNavigationButtons();
    void connectSignals();

    QList<QPair<QString, QString>> m_cards;
    QList<QPair<QString, QString>> m_originalCards;
    QList<bool> m_answerResults;

    // UI элементы
    QLabel* wordLabel;
    QLabel* resultLabel;
    QLabel* statsLabel;
    QLineEdit* answerLineEdit;
    QPushButton* prevButton;
    QPushButton* nextButton;
    QPushButton* checkButton;
    QPushButton* showAnswerButton;
    QPushButton* randomButton;
    QPushButton* resetOrderButton;
    QHBoxLayout* buttonsLayout;

    int m_currentIndex = 0;
    int m_correctAnswers = 0;
    bool m_answerChecked = false;
    bool m_showAnswer = false;
    bool m_forwardDirection = true;
};

#endif // LEARNINGMODEWINDOW_H

