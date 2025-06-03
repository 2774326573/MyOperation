#include <QApplication>

#include "inc/ui/mainwindow.h"

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  // 设置应用组织名称
  Mainwindow w;
  w.show();
  return QApplication::exec();
}
