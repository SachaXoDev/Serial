#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registrationwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_registationPushButton_clicked()
{
    this->hide();
    RegistrationWindow window;
    window.setModal(true);
    connect(&window, &RegistrationWindow::cancelPushButton, this, &MainWindow::show);
    window.exec();
}


void MainWindow::on_ExistPushButton_clicked()
{
    this->close();
}

