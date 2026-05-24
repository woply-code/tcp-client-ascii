#include "TcpClientWindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QStyle>
#include <QTcpSocket>

TcpClientWindow::TcpClientWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui_TcpClientWindow),
      socket(new QTcpSocket(this))
{
    ui->setupUi(this);
    setFixedSize(920, 560);

    // 布局居中拉伸
    ui->horizontalLayout->insertStretch(0, 1);
    ui->horizontalLayout->addStretch(1);

    // 窗口与文本设置（直接写中文，不会乱码）
    setWindowTitle("TCP 客户端");
    ui->titleLabel->setText("TCP 客户端");
    ui->subtitleLabel->setText("可视化 TCP 连接与控制面板");
    ui->connectionGroupBox->setTitle("连接设置");
    ui->controlGroupBox->setTitle("操作区");
    ui->ipLabel->setText("IP 地址");
    ui->portLabel->setText("端口");
    ui->connectButton->setText("连接");
    ui->openButton->setText("打开");
    ui->closeButton->setText("关闭");
    ui->statusTitleLabel->setText("连接状态");

    // 输入框提示与限制
    ui->ipLineEdit->setPlaceholderText("请输入服务器 IP");
    ui->portLineEdit->setPlaceholderText("请输入端口");
    ui->portLineEdit->setValidator(new QIntValidator(0, 65535, this));

    // 状态灯初始样式
    ui->statusDot->setStyleSheet("background-color: #93a4b7; border-radius: 7px;");
    updateStatus("未连接");

    // TCP 信号槽绑定
    connect(socket, &QTcpSocket::connected, this, &TcpClientWindow::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClientWindow::onSocketDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpClientWindow::onSocketErrorOccurred);
}

TcpClientWindow::~TcpClientWindow()
{
    delete ui;
}

void TcpClientWindow::on_connectButton_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
        return;
    }

    const quint16 port = ui->portLineEdit->text().toUShort();
    updateStatus(QStringLiteral("\u6b63\u5728\u8fde\u63a5..."));
    socket->connectToHost(ui->ipLineEdit->text(), port);
}

void TcpClientWindow::onSocketConnected()
{
    ui->connectButton->setText(QStringLiteral("\u65ad\u5f00\u8fde\u63a5"));
    updateStatus(QStringLiteral("TCP \u8fde\u63a5\u6210\u529f"));
}

void TcpClientWindow::onSocketDisconnected()
{
    ui->connectButton->setText(QStringLiteral("\u8fde\u63a5"));
    updateStatus(QStringLiteral("TCP \u5df2\u65ad\u5f00"));
}

void TcpClientWindow::onSocketErrorOccurred()
{
    ui->connectButton->setText(QStringLiteral("\u8fde\u63a5"));
    updateStatus(QStringLiteral("\u8fde\u63a5\u5931\u8d25\uff1a") + socket->errorString());
}

void TcpClientWindow::updateStatus(const QString &text)
{
    if (text.contains(QStringLiteral("\u6210\u529f"))) {
        updateStatusStyle(QStringLiteral("success"));
    } else if (text.contains(QStringLiteral("\u5931\u8d25"))) {
        updateStatusStyle(QStringLiteral("error"));
    } else if (text.contains(QStringLiteral("\u6b63\u5728"))) {
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

    ui->statusDot->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 7px;").arg(dotColor));
}
