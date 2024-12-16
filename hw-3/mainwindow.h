#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/evp.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openCertificate(); // Открыть файл сертификата
    void openRequest();     // Открыть файл запроса
    void signCertificate(); // Подписать запрос сертификата

private:
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QLineEdit *certificateLineEdit;
    QLineEdit *requestLineEdit;
    QPushButton *openCertificateButton;
    QPushButton *openRequestButton; // Новая кнопка для выбора файла запроса
    QPushButton *signButton;

    X509* loadCertificate(const QString &filePath); // Загрузить сертификат
    EVP_PKEY* loadPrivateKey(const QString &filePath); // Загрузить закрытый ключ
    bool signCertificateRequest(const QString &requestFilePath, const QString &signedFilePath, X509 *signerCert, EVP_PKEY *privateKey); // Реальная подпись CSR
};

#endif // MAINWINDOW_H
