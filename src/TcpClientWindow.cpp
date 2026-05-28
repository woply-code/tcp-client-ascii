#include "TcpClientWindow.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QIntValidator>
#include <QLineEdit>
#include <QSettings>
#include <QStyle>
#include <QTcpSocket>
#include <QTimer>

TcpClientWindow::TcpClientWindow(QWidget *parent)
    : QMainWindow(parent),
      selectionWindow(nullptr),
      ui(new Ui_TcpClientWindow),
      clientSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    setFixedSize(1200, 700);

    ui->portLineEdit->setValidator(new QIntValidator(0, 65535, this));
    connect(ui->sendLineEdit, &QLineEdit::returnPressed, this, &TcpClientWindow::on_sendButton_clicked);

    QSettings settings(QStringLiteral("QtTcpUiNew"), QStringLiteral("TcpTool"));
    ui->ipLineEdit->setText(settings.value(QStringLiteral("client/ip")).toString());
    ui->portLineEdit->setText(settings.value(QStringLiteral("client/port")).toString());

    connect(clientSocket, &QTcpSocket::connected, this, &TcpClientWindow::onSocketConnected);
    connect(clientSocket, &QTcpSocket::disconnected, this, &TcpClientWindow::onSocketDisconnected);
    connect(clientSocket,
            &QTcpSocket::errorOccurred,
            this,
            [this](QAbstractSocket::SocketError) { onSocketErrorOccurred(); });

    updateConnectButton();
    updateStatus(QStringLiteral("客户端就绪，等待发起连接"));
}

TcpClientWindow::~TcpClientWindow()
{
    delete ui;
}

void TcpClientWindow::setSelectionWindow(QWidget *window)
{
    selectionWindow = window;
}

void TcpClientWindow::on_connectButton_clicked()
{
    if (clientSocket->state() == QAbstractSocket::ConnectedState ||
        clientSocket->state() == QAbstractSocket::ConnectingState) {
        clientSocket->disconnectFromHost();
        if (clientSocket->state() == QAbstractSocket::ConnectingState) {
            clientSocket->abort();
        }
        updateConnectButton();
        return;
    }

    const QString ip = ui->ipLineEdit->text().trimmed();
    const QString portText = ui->portLineEdit->text().trimmed();

    QSettings settings(QStringLiteral("QtTcpUiNew"), QStringLiteral("TcpTool"));
    settings.setValue(QStringLiteral("client/ip"), ip);
    settings.setValue(QStringLiteral("client/port"), portText);

    const quint16 port = portText.toUShort();
    updateStatus(QStringLiteral("正在连接目标主机..."));
    updateConnectButton();
    clientSocket->connectToHost(ip, port);
}

void TcpClientWindow::on_backButton_clicked()
{
    if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
        clientSocket->disconnectFromHost();
        if (clientSocket->state() == QAbstractSocket::ConnectingState) {
            clientSocket->abort();
        }
    }
    returnToSelection();
}

void TcpClientWindow::onSocketConnected()
{
    updateStatus(QStringLiteral("TCP 客户端连接成功"));
    updateConnectButton();
}

void TcpClientWindow::onSocketDisconnected()
{
    updateStatus(QStringLiteral("TCP 客户端已断开"));
    updateConnectButton();
}

void TcpClientWindow::onSocketErrorOccurred()
{
    updateStatus(QStringLiteral("连接失败：") + clientSocket->errorString());
    updateConnectButton();
}

bool TcpClientWindow::ensureSocketConnected()
{
    if (clientSocket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    updateStatus(QStringLiteral("请先进行连接"));
    QTimer::singleShot(5000, this, [this]() {
        updateStatus(QStringLiteral("客户端就绪，等待发起连接"));
    });
    return false;
}

QByteArray TcpClientWindow::buildSendPayload(bool *ok, QString *errorMessage) const
{
    const QString encoding = ui->encodingComboBox->currentText();
    const QString input = ui->sendLineEdit->text();

    if (input.isEmpty()) {
        *ok = false;
        *errorMessage = QStringLiteral("发送内容不能为空");
        return {};
    }

    if (encoding == QStringLiteral("UTF-8")) {
        *ok = true;
        return input.toUtf8();
    }

    if (encoding == QStringLiteral("US-ASCII")) {
        QByteArray payload;
        payload.reserve(input.size());

        for (const QChar ch : input) {
            if (ch.unicode() > 0x7f) {
                *ok = false;
                *errorMessage = QStringLiteral("US-ASCII 模式仅支持 0x00-0x7F 字符");
                return {};
            }
            payload.append(static_cast<char>(ch.unicode()));
        }

        *ok = true;
        return payload;
    }

    QString normalizedHex;
    normalizedHex.reserve(input.size());

    for (const QChar ch : input) {
        if (ch.isSpace()) {
            continue;
        }

        const ushort value = ch.unicode();
        const bool isHexDigit = (value >= '0' && value <= '9') ||
                                (value >= 'a' && value <= 'f') ||
                                (value >= 'A' && value <= 'F');
        if (!isHexDigit) {
            *ok = false;
            *errorMessage = QStringLiteral("Hex 模式仅支持十六进制字符和空格");
            return {};
        }

        normalizedHex.append(ch);
    }

    if (normalizedHex.isEmpty()) {
        *ok = false;
        *errorMessage = QStringLiteral("Hex 内容不能为空");
        return {};
    }

    if (normalizedHex.size() % 2 != 0) {
        *ok = false;
        *errorMessage = QStringLiteral("Hex 字节数必须是偶数位");
        return {};
    }

    *ok = true;
    return QByteArray::fromHex(normalizedHex.toLatin1());
}

void TcpClientWindow::updateConnectButton()
{
    if (clientSocket->state() == QAbstractSocket::ConnectedState ||
        clientSocket->state() == QAbstractSocket::ConnectingState) {
        ui->connectButton->setText(QStringLiteral("断开连接"));
    } else {
        ui->connectButton->setText(QStringLiteral("连接"));
    }
}

void TcpClientWindow::updateStatus(const QString &text)
{
    if (text.contains(QStringLiteral("成功")) || text.contains(QStringLiteral("已发送"))) {
        updateStatusStyle(QStringLiteral("success"));
    } else if (text.contains(QStringLiteral("失败")) ||
               text.contains(QStringLiteral("异常")) ||
               text.contains(QStringLiteral("请先进行连接")) ||
               text.contains(QStringLiteral("不能为空")) ||
               text.contains(QStringLiteral("仅支持")) ||
               text.contains(QStringLiteral("偶数位")) ||
               text.contains(QStringLiteral("不完整"))) {
        updateStatusStyle(QStringLiteral("error"));
    } else if (text.contains(QStringLiteral("正在"))) {
        updateStatusStyle(QStringLiteral("busy"));
    } else {
        updateStatusStyle(QStringLiteral("idle"));
    }

    ui->statusLabel->setText(text);
}

void TcpClientWindow::updateStatusStyle(const QString &state)
{
    ui->statusLabel->setProperty("state", state);
    ui->statusLabel->style()->unpolish(ui->statusLabel);
    ui->statusLabel->style()->polish(ui->statusLabel);
    ui->statusLabel->update();

    QString dotColor = QStringLiteral("#93a4b7");
    if (state == QStringLiteral("success")) {
        dotColor = QStringLiteral("#34a35f");
    } else if (state == QStringLiteral("error")) {
        dotColor = QStringLiteral("#d26a56");
    } else if (state == QStringLiteral("busy")) {
        dotColor = QStringLiteral("#4f86f7");
    }

    ui->statusDot->setStyleSheet(
        QStringLiteral("background-color: %1; border-radius: 7px;").arg(dotColor));
}

void TcpClientWindow::returnToSelection()
{
    hide();
    if (!selectionWindow) {
        return;
    }

    selectionWindow->show();
    selectionWindow->raise();
    selectionWindow->activateWindow();
}

void TcpClientWindow::on_openButton_clicked()
{
    if (!ensureSocketConnected()) {
        return;
    }

    clientSocket->write("OPEN\n");
    clientSocket->flush();
    updateStatus(QStringLiteral("已发送 OPEN 指令"));
}

void TcpClientWindow::on_closeButton_clicked()
{
    if (!ensureSocketConnected()) {
        return;
    }

    clientSocket->write("CLOSE\n");
    clientSocket->flush();
    updateStatus(QStringLiteral("已发送 CLOSE 指令"));
}

void TcpClientWindow::on_sendButton_clicked()
{
    if (!ensureSocketConnected()) {
        return;
    }

    bool ok = false;
    QString errorMessage;
    const QByteArray payload = buildSendPayload(&ok, &errorMessage);
    if (!ok) {
        updateStatus(errorMessage);
        return;
    }

    const qint64 writtenBytes = clientSocket->write(payload);
    clientSocket->flush();

    if (writtenBytes != payload.size()) {
        updateStatus(QStringLiteral("发送失败：写入字节数不完整"));
        return;
    }

    updateStatus(QStringLiteral("已发送 %1 字节（%2）")
                     .arg(payload.size())
                     .arg(ui->encodingComboBox->currentText()));
}
