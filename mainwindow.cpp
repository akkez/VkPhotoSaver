#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->chooseGroupBox->setVisible(false);
    ui->dowloadGroupBox->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}
