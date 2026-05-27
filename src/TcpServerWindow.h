#pragma once

#include "ui_TcpServerWindow.h"
#include <QAbstractSocket>
#include <QMainWindow>

class QTcpServer;
class QTcpSocket;

class TcpServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TcpServerWindow(QWidget *parent = nullptr);
    ~TcpServerWindow();

    void setSelectionWindow(QWidget *window);

private slots:
    void on_listenButton_clicked();
    void on_backButton_clicked();
    void onNewConnection();
    void onClientDisconnected();
    void onClientErrorOccurred();
    void onServerAcceptError(QAbstractSocket::SocketError socketError);

private:
    void startListening();
    void stopListening();
    void closeActiveClientSocket();
    void attachClientSocket(QTcpSocket *newSocket);
    void updateListenButton();
    void updateStatus(const QString &text);
    void updateStatusStyle(const QString &state);
    void returnToSelection();

    QWidget *selectionWindow;
    Ui_TcpServerWindow *ui;
    QTcpServer *server;
    QTcpSocket *clientSocket;
};
