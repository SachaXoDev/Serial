#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QList<QPair<QString, QString>> getCardsForModule(int moduleId);
    QList<QPair<QString, QString>> getShuffledCardsForModule(int moduleId);
    bool addCardToModule(int moduleId, const QString &word, const QString &translation);
    bool updateCardInModule(int moduleId, const QString &oldWord, const QString &oldTranslation,
                            const QString &newWord, const QString &newTranslation);
    bool deleteCardFromModule(int moduleId, const QString &word, const QString &translation);
    static bool initializeDatabase();

private slots:
    void on_registationPushButton_clicked();
    void handleUserRegistration(const QString &username, const QString &password);
    void on_ExistPushButton_clicked();
    void on_OkpushButton_clicked();

private:
    Ui::MainWindow *ui;
    int currentUserId = -1;

    static QString getDatabasePath();
    void connectToDatabase();
    void createTables();
    void showUsers();
    void insertUser(const QString &username, const QString &password);
    void showModulesForCurrentUser();
    bool moduleExists(int userId, const QString &moduleName);
    void deleteModule(int userId, const QString &moduleName);
};

#endif // MAINWINDOW_H
