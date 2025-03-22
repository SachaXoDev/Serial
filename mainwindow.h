#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_registationPushButton_clicked();
    void on_ExistPushButton_clicked();
    void handleUserRegistration(const QString &username, const QString &password);

    void on_OkpushButton_clicked();

private:
    Ui::MainWindow *ui;
    void connectToDatabase();
    void createTables();
    void insertUser(const QString &username, const QString &password);
    void showUsers();
};

#endif // MAINWINDOW_H
