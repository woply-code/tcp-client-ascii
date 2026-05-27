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

    void setSelectionWindow(QWidget *window);

private slots:
    void on_connectButton_clicked();
    void on_backButton_clicked();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketErrorOccurred();

private:
    void updateConnectButton();
    void updateStatus(const QString &text);
    void updateStatusStyle(const QString &state);
    void returnToSelection();

    QWidget *selectionWindow;
    Ui_TcpClientWindow *ui;
    QTcpSocket *clientSocket;
};
