#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Certificate Request Signer");
    window.resize(400, 200); // Устанавливаем размеры окна
    window.show();

    return app.exec();
}
