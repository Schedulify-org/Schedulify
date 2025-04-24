#include "../include/mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int firstNum, secondNum;

void MainWindow::extractNumbers()
{
    firstNum = ui->etFirstNum->text().toInt();
    secondNum = ui->etSecondNum->text().toInt();
}

void MainWindow::on_btPlus_clicked()
{
    MainWindow::extractNumbers();
    ui->etResult->setText(QString::number(firstNum + secondNum));
}


void MainWindow::on_btMinus_clicked()
{
    MainWindow::extractNumbers();
    ui->etResult->setText(QString::number(firstNum - secondNum));
}


void MainWindow::on_btDiv_clicked()
{
    MainWindow::extractNumbers();
    ui->etResult->setText(QString::number(firstNum / secondNum));
}


void MainWindow::on_btMult_clicked()
{
    MainWindow::extractNumbers();
    ui->etResult->setText(QString::number(firstNum * secondNum));
}

