#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QString>
#include <QFile>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/err.h>
#include <QInputDialog>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();

private:
    Ui::MainWindow *ui;
    void processCertificate(const QString& filename);
    QString getCertificateInfo(X509 *cert);
    QString getPKCS7Info(PKCS7 *p7);
    QString getPKCS12Info(PKCS12 *p12, const QString& password);
};

#endif // MAINWINDOW_H
