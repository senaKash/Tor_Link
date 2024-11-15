#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "TorController.h"  // Подключаем заголовок класса TorController

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Класс для главного окна приложения
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);  // Конструктор
    ~MainWindow();  // Деструктор

private slots:
    void onConnectButtonClicked();  // Слот для обработки нажатия кнопки
    void onTorLogUpdated(const QString &log);  // Слот для обновления лога
    void onConnectedToNetwork(bool isConnected);  // Слот для обновления статуса сети

private:
    Ui::MainWindow *ui;  // Интерфейс пользователя
    TorController *torController;  // Контроллер для управления Tor
};

#endif // MAINWINDOW_H
