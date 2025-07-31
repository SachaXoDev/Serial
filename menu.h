#ifndef MENU_H
#define MENU_H

#include <QDialog>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QSqlError>
#include <QPainterPath>

namespace Ui {
class Menu;
}

class MainWindow;

class Menu : public QDialog
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr, MainWindow *mainWindow = nullptr);
    ~Menu();

    void setUserId(int id);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_exitPush_clicked();
    void on_addModuleButton_clicked();
    void deleteModule();
    void showCardsManagementDialog(int moduleId, const QString &moduleName);
    void startLearningMode(int moduleId);
    void on_favoriteButton_clicked();

signals:
    void exitPush();

private:
    Ui::Menu *ui;
    int userId = -1;
    bool m_isDialogOpen = false;
    QVBoxLayout *modulesLayout;
    MainWindow *m_mainWindow;

    void setupUi();
    void loadModulesFromDb();
    void addModuleToUi(int moduleId, const QString &moduleName);
};

#endif // MENU_H

