#ifndef TORCONTROLLER_H
#define TORCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>

class TorController : public QObject
{
    Q_OBJECT

public:
    explicit TorController(QObject *parent = nullptr);
    ~TorController();

    void startTor(const QString &torrcPath, const QString &bridgeFilePath); // Сохраняем пути к torrc и файлу мостов
    void stopTor();

signals:
    void logUpdated(const QString &log);
    void connectedToNetwork(bool connected);

private slots:
    void onTorLogUpdated();
    void onTorFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCheckTorReply(QNetworkReply *reply);

private:
    bool loadBridgesFromFile(const QString &bridgeFilePath);
    void killProcessOnPort(int port);
    void attemptToStartTor();  // Используем сохраненный путь для конфигурации torrc
    void addBridgeToTorrc(const QString &torrcPath, const QString &bridge);
    void setApplicationProxy();
    void checkTorConnection();

    QProcess *torProcess;
    QNetworkAccessManager *networkManager;

    QStringList bridgesList;
    int currentBridgeIndex = 0;

    // Добавлено: Переменные для хранения путей
    QString savedTorrcPath; // Путь к файлу torrc
    QString savedBridgeFilePath; // Путь к файлу мостов
};

#endif // TORCONTROLLER_H
