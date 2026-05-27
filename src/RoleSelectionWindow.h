#pragma once

#include "ui_RoleSelectionWindow.h"
#include <QWidget>

class TcpClientWindow;
class TcpServerWindow;

class RoleSelectionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RoleSelectionWindow(QWidget *parent = nullptr);
    ~RoleSelectionWindow();

private slots:
    void on_clientButton_clicked();
    void on_serverButton_clicked();

private:
    void openClientWindow();
    void openServerWindow();

    Ui_RoleSelectionWindow *ui;
    TcpClientWindow *clientWindow;
    TcpServerWindow *serverWindow;
};
