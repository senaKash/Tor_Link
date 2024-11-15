#include "TorController.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <iostream>
#include <QRegularExpression>

TorController::TorController(QObject *parent)
    : QObject(parent),
      torProcess(new QProcess(this)),
      networkManager(new QNetworkAccessManager(this))
{
    connect(torProcess, &QProcess::readyReadStandardOutput, this, &TorController::onTorLogUpdated);
    connect(torProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &TorController::onTorFinished);
}

TorController::~TorController()
{
    stopTor();
}

void TorController::startTor(const QString &torrcPath, const QString &bridgeFilePath)
{
    // Сохраняем пути к файлам
    savedTorrcPath = "torrc";
    savedBridgeFilePath = "mosti.txt";

    if (!loadBridgesFromFile(savedBridgeFilePath)) {
        emit logUpdated("Не удалось загрузить мосты из файла: " + savedBridgeFilePath);
        return;
    }

    currentBridgeIndex = 0;
    attemptToStartTor();  // Теперь вызываем без аргументов, использует сохраненные пути
}


void TorController::killProcessOnPort(int port)
{
    // Выполняем команду netstat для поиска процесса, использующего порт
    QProcess netstatProcess;
    netstatProcess.start("netstat", QStringList() << "-ano" << "| findstr" << QString::number(port));
    netstatProcess.waitForFinished();
    QString output = netstatProcess.readAllStandardOutput();

    // Ищем PID (последняя колонка вывода)
    QStringList lines = output.split("\n", QString::SkipEmptyParts);
    if (lines.isEmpty()) {
        emit logUpdated("Процесс на порту " + QString::number(port) + " не найден.");
        return;
    }

    QStringList columns = lines.first().split(" ", QString::SkipEmptyParts);
    QString pid = columns.last(); // PID процесса

    emit logUpdated("Процесс на порту " + QString::number(port) + " имеет PID: " + pid);

    // Убиваем процесс по PID
    QProcess killProcess;
    killProcess.start("taskkill", QStringList() << "/PID" << pid << "/F");
    killProcess.waitForFinished();
    emit logUpdated("Процесс с PID: " + pid + " был завершен.");
}



void TorController::attemptToStartTor()
{
    // Убиваем процесс, использующий порт 9050, если он существует
    killProcessOnPort(9050);

    // Проверяем, есть ли доступные мосты
    if (currentBridgeIndex >= bridgesList.size()) {
        emit logUpdated("Все мосты испробованы, нет доступных для подключения.");
        return;
    }

    QString bridge = bridgesList.at(currentBridgeIndex);
    addBridgeToTorrc(savedTorrcPath, bridge); // Создаем новый torrc

    // Запускаем Tor с новым мостом
    torProcess->start("tor.exe", QStringList() << "-f" << savedTorrcPath);

    if (!torProcess->waitForStarted()) {
        emit logUpdated("Не удалось запустить tor.exe");
        return;
    }

    emit logUpdated("Попытка подключения через мост: " + bridge);
}

bool TorController::loadBridgesFromFile(const QString &bridgeFilePath)
{
    QFile file(bridgeFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Пропускаем пустые строки
        if (line.isEmpty()) {
            continue;
        }

        // Добавляем строку моста в список
        bridgesList.append(line);
    }

    file.close();
    return !bridgesList.isEmpty();
}

void TorController::stopTor()
{
    if (torProcess->state() == QProcess::Running) {
        torProcess->terminate();
        torProcess->waitForFinished();
    }
}

void TorController::addBridgeToTorrc(const QString &torrcPath, const QString &bridge)
{
    QFile torrc(torrcPath);
    if (!torrc.open(QIODevice::ReadWrite | QIODevice::Text)) {
        emit logUpdated("Не удалось открыть файл torrc");
        return;
    }

    QTextStream in(&torrc);
    QString torrcContent = in.readAll();

    torrc.seek(0);
    QTextStream out(&torrc);

    out << torrcContent;
    out << "\nUseBridges 1\n";

    // Разделяем строку моста на IP:порт и идентификатор
    QStringList bridgeParts = bridge.split(" ");
    if (bridgeParts.size() == 2) {
        QString bridgeAddress = bridgeParts.at(0);
        QString bridgeFingerprint = bridgeParts.at(1);

        // Записываем мост в файл torrc в правильном формате
        out << "Bridge " << bridgeAddress << " " << bridgeFingerprint << "\n";
    } else {
        emit logUpdated("Неверный формат моста: " + bridge);
    }

    torrc.close();
}

void TorController::onTorLogUpdated()
{
    QString output = torProcess->readAllStandardOutput();
    emit logUpdated(output);

    if (output.contains("Bootstrapped 100%")) {
        emit connectedToNetwork(true);
        setApplicationProxy();
        checkTorConnection();
    } else if (output.contains("Problem bootstrapping")) {
        emit logUpdated("Проблема с подключением через мост: " + bridgesList.at(currentBridgeIndex));
        currentBridgeIndex++;
        stopTor();
        attemptToStartTor(); // Пробуем следующий мост
    }
}

void TorController::setApplicationProxy()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(9050);

    QNetworkProxy::setApplicationProxy(proxy);
    qDebug() << "Прокси установлен: 127.0.0.1:9050";
}

void TorController::checkTorConnection()
{
    QNetworkRequest request(QUrl("https://check.torproject.org/"));
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCheckTorReply(reply);
    });
}

void TorController::onCheckTorReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        if (response.contains("Congratulations. This browser is configured to use Tor.")) {
            emit logUpdated("Вы подключены через Tor.");
            emit connectedToNetwork(true);
        } else {
            emit logUpdated("Вы не используете Tor.");
            emit connectedToNetwork(false);
        }
    } else {
        emit logUpdated("Ошибка при проверке подключения к Tor: " + reply->errorString());
        emit connectedToNetwork(false);
    }
    reply->deleteLater();
}

void TorController::onTorFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    emit logUpdated("Tor завершил работу");
}
