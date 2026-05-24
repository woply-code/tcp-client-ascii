#pragma once

#include "ui_TcpClientWindow.h"
#include <QMainWindow>

class QTcpSocket;

class TcpClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TcpClientWindow(QWidget *parent = nullptr);
    ~TcpClientWindow();

private slots:
    void on_connectButton_clicked();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketErrorOccurred();

private:
    void updateStatus(const QString &text);
    void updateStatusStyle(const QString &state);

    Ui_TcpClientWindow *ui;
    QTcpSocket *socket;
};
