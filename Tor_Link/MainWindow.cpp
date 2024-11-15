#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , torController(new TorController(this))
{
    ui->setupUi(this);

    connect(ui->pushButton_connect, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(torController, &TorController::logUpdated, this, &MainWindow::onTorLogUpdated);
    connect(torController, &TorController::connectedToNetwork, this, &MainWindow::onConnectedToNetwork);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectButtonClicked()
{
    QString bridge = ui->lineEdit_bridge->text();

    ui->lineEdit_bridge->setDisabled(true);
    ui->pushButton_connect->setDisabled(true);

    QString torrcPath = "torrc";
    torController->startTor(torrcPath, bridge);
}

void MainWindow::onTorLogUpdated(const QString &log)
{
    ui->textEdit_log->append(log);
}

void MainWindow::onConnectedToNetwork(bool isConnected)
{
    if (isConnected) {
        ui->label_torstatus->setText("В сети");
    } else {
        ui->label_torstatus->setText("Не в сети");
    }
}
