#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OPENSSL_init_crypto(0, NULL);
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    MainWindow w;
    w.show();
    return a.exec();
}
