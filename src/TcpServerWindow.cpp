#include "TcpServerWindow.h"

#include <QHostAddress>
#include <QIntValidator>
#include <QSettings>
#include <QStyle>
#include <QTcpServer>
#include <QTcpSocket>

// 初始化服务端窗口，并绑定监听与连接相关信号。
TcpServerWindow::TcpServerWindow(QWidget *parent)
    : QMainWindow(parent),
      selectionWindow(nullptr),
      ui(new Ui_TcpServerWindow),
      server(new QTcpServer(this)),
      clientSocket(nullptr)
{
    ui->setupUi(this);
    setFixedSize(920, 560);

    ui->portLineEdit->setValidator(new QIntValidator(0, 65535, this));

    QSettings settings(QStringLiteral("QtTcpUiNew"), QStringLiteral("TcpTool"));
    ui->portLineEdit->setText(settings.value(QStringLiteral("server/port")).toString());

    connect(server, &QTcpServer::newConnection, this, &TcpServerWindow::onNewConnection);
    connect(server, &QTcpServer::acceptError, this, &TcpServerWindow::onServerAcceptError);

    updateListenButton();
    updateStatus(QStringLiteral("服务端就绪，等待开始监听"));
}

// 释放服务端窗口的界面资源。
TcpServerWindow::~TcpServerWindow()
{
    delete ui;
}

// 记录首页窗口指针，便于返回选择页。
void TcpServerWindow::setSelectionWindow(QWidget *window)
{
    selectionWindow = window;
}

// 处理监听按钮点击：已监听则停止，未监听则开始。
void TcpServerWindow::on_listenButton_clicked()
{
    if (server->isListening()) {
        stopListening();
        return;
    }

    startListening();
}

// 处理返回按钮点击，并回到首页选择页。
void TcpServerWindow::on_backButton_clicked()
{
    stopListening();
    returnToSelection();
}

// 处理新的客户端接入请求。
void TcpServerWindow::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket *pendingSocket = server->nextPendingConnection();
        if (!pendingSocket) {
            continue;
        }

        if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
            pendingSocket->disconnectFromHost();
            pendingSocket->deleteLater();
            updateStatus(QStringLiteral("已有客户端接入，已拒绝新的连接请求"));
            continue;
        }

        attachClientSocket(pendingSocket);
    }
}

// 处理已接入客户端断开连接的情况。
void TcpServerWindow::onClientDisconnected()
{
    auto *currentSocket = qobject_cast<QTcpSocket *>(sender());
    if (!currentSocket) {
        return;
    }

    updateStatus(QStringLiteral("接入客户端已断开，服务端继续监听中"));
    currentSocket->deleteLater();
    if (clientSocket == currentSocket) {
        clientSocket = nullptr;
    }
    updateListenButton();
}

// 处理已接入客户端的异常信息。
void TcpServerWindow::onClientErrorOccurred()
{
    auto *currentSocket = qobject_cast<QTcpSocket *>(sender());
    if (!currentSocket) {
        return;
    }

    updateStatus(QStringLiteral("客户端连接异常：") + currentSocket->errorString());
    updateListenButton();
}

// 处理服务端监听失败的情况。
void TcpServerWindow::onServerAcceptError(QAbstractSocket::SocketError)
{
    updateStatus(QStringLiteral("监听失败：") + server->errorString());
    updateListenButton();
}

// 启动服务端监听，并更新界面状态。
void TcpServerWindow::startListening()
{
    closeActiveClientSocket();

    const QString portText = ui->portLineEdit->text().trimmed();

    QSettings settings(QStringLiteral("QtTcpUiNew"), QStringLiteral("TcpTool"));
    settings.setValue(QStringLiteral("server/port"), portText);

    const quint16 port = portText.toUShort();
    updateStatus(QStringLiteral("正在启动服务端监听..."));

    if (!server->listen(QHostAddress::Any, port)) {
        updateStatus(QStringLiteral("监听失败：") + server->errorString());
        updateListenButton();
        return;
    }

    updateStatus(QStringLiteral("服务端监听中，等待客户端连接：0.0.0.0:%1").arg(port));
    updateListenButton();
}

// 停止服务端监听，并关闭当前客户端连接。
void TcpServerWindow::stopListening()
{
    closeActiveClientSocket();
    if (server->isListening()) {
        server->close();
    }
    updateStatus(QStringLiteral("服务端监听已停止"));
    updateListenButton();
}

// 关闭当前已接入的客户端连接。
void TcpServerWindow::closeActiveClientSocket()
{
    if (!clientSocket) {
        return;
    }

    disconnect(clientSocket, nullptr, this, nullptr);
    if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
        clientSocket->disconnectFromHost();
        clientSocket->abort();
    }
    clientSocket->deleteLater();
    clientSocket = nullptr;
}

// 绑定新的客户端连接，并更新接入状态。
void TcpServerWindow::attachClientSocket(QTcpSocket *newSocket)
{
    closeActiveClientSocket();

    clientSocket = newSocket;
    connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServerWindow::onClientDisconnected);
    connect(clientSocket,
            &QTcpSocket::errorOccurred,
            this,
            [this](QAbstractSocket::SocketError) { onClientErrorOccurred(); });

    updateStatus(QStringLiteral("客户端已接入：%1:%2")
                     .arg(clientSocket->peerAddress().toString())
                     .arg(clientSocket->peerPort()));
    updateListenButton();
}

// 根据监听状态刷新监听按钮文字。
void TcpServerWindow::updateListenButton()
{
    ui->listenButton->setText(server->isListening() ? QStringLiteral("停止监听")
                                                    : QStringLiteral("开始监听"));
}

// 根据状态文本更新状态标签内容和样式类型。
void TcpServerWindow::updateStatus(const QString &text)
{
    if (text.contains(QStringLiteral("成功")) || text.contains(QStringLiteral("已接入")) ||
        text.contains(QStringLiteral("监听中"))) {
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
void TcpServerWindow::updateStatusStyle(const QString &state)
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
void TcpServerWindow::returnToSelection()
{
    hide();
    if (!selectionWindow) {
        return;
    }

    selectionWindow->show();
    selectionWindow->raise();
    selectionWindow->activateWindow();
}
