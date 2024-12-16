#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <QFileDialog>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_createButton_clicked();

private:
    Ui::MainWindow *ui;

    // Функции создания сертификатов и ключей
    X509 *createCertificate(const QMap<QString, QString> &subjectInfo, EVP_PKEY *privateKey);
    EVP_PKEY *createPrivateKey();

    // Функции сохранения сертификатов в различных форматах
    void saveCertificatePEM(X509 *cert, EVP_PKEY *key, const QString &filePath);
    void saveCertificateCRT(X509 *cert, const QString &filePath);
    void saveCertificateP7B(X509 *cert, const QString &filePath);
    void saveCertificatePFX(X509 *cert, EVP_PKEY *key, const QString &filePath);
    void saveCertificateCSR(X509 *cert, const QString &filePath);
};

#endif // MAINWINDOW_H
