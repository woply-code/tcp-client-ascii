#include "RoleSelectionWindow.h"
#include <QApplication>

// 程序入口：初始化应用并显示角色选择页。
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RoleSelectionWindow window;
    window.show();

    return app.exec();
}
