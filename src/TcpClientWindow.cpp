#include "TcpClientWindow.h"

#include <QAbstractSocket>
#include <QIntValidator>
#include <QSettings>
#include <QStyle>
#include <QTcpSocket>

// 初始化客户端窗口，并绑定客户端连接相关信号。
TcpClientWindow::TcpClientWindow(QWidget *parent)
    : QMainWindow(parent),
      selectionWindow(nullptr),
      ui(new Ui_TcpClientWindow),
      clientSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    setFixedSize(920, 560);

    ui->portLineEdit->setValidator(new QIntValidator(0, 65535, this));

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

// 释放客户端窗口的界面资源。
TcpClientWindow::~TcpClientWindow()
{
    delete ui;
}

// 记录首页窗口指针，便于返回选择页。
void TcpClientWindow::setSelectionWindow(QWidget *window)
{
    selectionWindow = window;
}

// 处理连接按钮点击：已连接则断开，未连接则发起连接。
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

// 处理返回按钮点击，并回到首页选择页。
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

// 客户端连接成功后更新状态显示。
void TcpClientWindow::onSocketConnected()
{
    updateStatus(QStringLiteral("TCP 客户端连接成功"));
    updateConnectButton();
}

// 客户端断开连接后更新状态显示。
void TcpClientWindow::onSocketDisconnected()
{
    updateStatus(QStringLiteral("TCP 客户端已断开"));
    updateConnectButton();
}

// 客户端连接异常时显示错误信息。
void TcpClientWindow::onSocketErrorOccurred()
{
    updateStatus(QStringLiteral("连接失败：") + clientSocket->errorString());
    updateConnectButton();
}

// 根据当前连接状态刷新连接按钮文字。
void TcpClientWindow::updateConnectButton()
{
    if (clientSocket->state() == QAbstractSocket::ConnectedState ||
        clientSocket->state() == QAbstractSocket::ConnectingState) {
        ui->connectButton->setText(QStringLiteral("断开连接"));
    } else {
        ui->connectButton->setText(QStringLiteral("连接"));
    }
}

// 根据状态文本更新状态标签内容和样式类型。
void TcpClientWindow::updateStatus(const QString &text)
{
    if (text.contains(QStringLiteral("成功"))) {
        updateStatusStyle(QStringLiteral("success"));
    } else if (text.contains(QStringLiteral("失败")) || text.contains(QStringLiteral("异常"))) {
        updateStatusStyle(QStringLiteral("error"));
    } else if (text.contains(QStringLiteral("正在"))) {
        updateStatusStyle(QStringLiteral("busy"));
    } else {
        updateStatusStyle(QStringLiteral("idle"));
    }

    ui->statusLabel->setText(text);
}

// 根据状态类型切换状态标签和状态点的视觉样式。
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

// 隐藏当前窗口，并重新显示首页选择页。
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
