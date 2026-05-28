#include "RoleSelectionWindow.h"

#include "TcpClientWindow.h"
#include "TcpServerWindow.h"

// 初始化首页选择窗口。
RoleSelectionWindow::RoleSelectionWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui_RoleSelectionWindow),
      clientWindow(nullptr),
      serverWindow(nullptr)
{
    ui->setupUi(this);
    setFixedSize(1200, 700);
}

// 释放选择窗口的界面资源。
RoleSelectionWindow::~RoleSelectionWindow()
{
    delete ui;
}

// 点击客户端按钮后进入客户端窗口。
void RoleSelectionWindow::on_clientButton_clicked()
{
    openClientWindow();
}

// 点击服务端按钮后进入服务端窗口。
void RoleSelectionWindow::on_serverButton_clicked()
{
    openServerWindow();
}

// 打开客户端窗口，并隐藏当前选择页。
void RoleSelectionWindow::openClientWindow()
{
    if (!clientWindow) {
        clientWindow = new TcpClientWindow();
        clientWindow->setSelectionWindow(this);
    }//如果客户端窗口没创建，创建一个tcp窗口

    clientWindow->show();
    clientWindow->raise();
    clientWindow->activateWindow();
    hide();
}

// 打开服务端窗口，并隐藏当前选择页。
void RoleSelectionWindow::openServerWindow()
{
    if (!serverWindow) {
        serverWindow = new TcpServerWindow();
        serverWindow->setSelectionWindow(this);
    }

    serverWindow->show();
    serverWindow->raise();
    serverWindow->activateWindow();
    hide();
}
