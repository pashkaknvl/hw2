#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/evp.h>
#include <QMessageBox>
#include <QMap>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_createButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_createButton_clicked() {
    QMap<QString, QString> subjectInfo;

    subjectInfo["C"] = ui->comboBox->currentText();
    subjectInfo["ST"] = ui->lineEdit_6->text();
    subjectInfo["L"] = ui->lineEdit_5->text();
    subjectInfo["O"] = ui->lineEdit_4->text();
    subjectInfo["OU"] = ui->lineEdit_3->text();
    subjectInfo["CN"] = ui->lineEdit_2->text();
    subjectInfo["emailAddress"] = ui->lineEdit->text();

    if (subjectInfo["CN"].isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Вы должны написат имя");
        return;
    }

    EVP_PKEY *privateKey = createPrivateKey();
    if (!privateKey) {
        QMessageBox::critical(this, "Ошибка", "Невозможно создать закрытый ключ");
        return;
    }

    X509 *cert = createCertificate(subjectInfo, privateKey);
    if (!cert) {
        QMessageBox::critical(this, "Ошибка", "Невозможно создать сертификат.");
        EVP_PKEY_free(privateKey);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить сертификат"), "", tr("Файл сертификта (*.pem *.crt *.cer *.p7b *.pfx *.csr)"));
    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".pem")) {
            saveCertificatePEM(cert, privateKey, fileName);
        } else if (fileName.endsWith(".crt") || fileName.endsWith(".cer")) {
            saveCertificateCRT(cert, fileName);
        } else if (fileName.endsWith(".p7b")) {
            saveCertificateP7B(cert, fileName);
        } else if (fileName.endsWith(".pfx")) {
            saveCertificatePFX(cert, privateKey, fileName);
        } else if (fileName.endsWith(".csr")) {
            saveCertificateCSR(cert, fileName);
        } else {
            QMessageBox::warning(this, "Ошибка", "Unsupported file format. Defaulting to .pem");
            saveCertificatePEM(cert, privateKey, fileName + ".pem");
        }
    }

    X509_free(cert);
    EVP_PKEY_free(privateKey);
}

X509 *MainWindow::createCertificate(const QMap<QString, QString> &subjectInfo, EVP_PKEY *privateKey) {
    X509 *cert = X509_new();
    if (!cert) return nullptr;

    X509_set_version(cert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);

    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);

    X509_NAME *name = X509_NAME_new();
    for (auto it = subjectInfo.begin(); it != subjectInfo.end(); ++it) {
        if (!it.value().isEmpty()) {
            X509_NAME_add_entry_by_txt(name, it.key().toUtf8().constData(), MBSTRING_ASC,
                                       (unsigned char *)it.value().toUtf8().constData(), -1, -1, 0);
        }
    }
    X509_set_subject_name(cert, name);
    X509_set_issuer_name(cert, name);
    X509_NAME_free(name);

    X509_set_pubkey(cert, privateKey);

    if (!X509_sign(cert, privateKey, EVP_sha256())) {
        X509_free(cert);
        return nullptr;
    }

    return cert;
}

EVP_PKEY *MainWindow::createPrivateKey() {
    EVP_PKEY *pkey = nullptr;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

    if (!ctx) return nullptr;

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}


void MainWindow::saveCertificatePEM(X509 *cert, EVP_PKEY *key, const QString &filePath) {
    BIO *bio = BIO_new_file(filePath.toUtf8().constData(), "w");
    if (!bio) return;

    PEM_write_bio_X509(bio, cert);
    PEM_write_bio_PrivateKey(bio, key, nullptr, nullptr, 0, nullptr, nullptr);
    BIO_free(bio);
}

void MainWindow::saveCertificateCRT(X509 *cert, const QString &filePath) {
    BIO *bio = BIO_new_file(filePath.toUtf8().constData(), "w");
    if (!bio) return;

    PEM_write_bio_X509(bio, cert);
    BIO_free(bio);
}

void MainWindow::saveCertificateP7B(X509 *cert, const QString &filePath) {
    STACK_OF(X509) *certs = sk_X509_new_null();
    sk_X509_push(certs, cert);

    BIO *bio = BIO_new_file(filePath.toUtf8().constData(), "w");
    if (!bio) return;

    PKCS7 *p7 = PKCS7_new();
    PKCS7_set_type(p7, NID_pkcs7_signed);
    PKCS7_content_new(p7, NID_pkcs7_data);
    PKCS7_add_certificate(p7, cert);

    PEM_write_bio_PKCS7(bio, p7);

    PKCS7_free(p7);
    BIO_free(bio);
}

void MainWindow::saveCertificatePFX(X509 *cert, EVP_PKEY *key, const QString &filePath) {
    PKCS12 *p12 = PKCS12_create("password", "My Certificate", key, cert, nullptr, 0, 0, 0, 0, 0);
    if (!p12) return;

    BIO *bio = BIO_new_file(filePath.toUtf8().constData(), "wb");
    if (!bio) {
        PKCS12_free(p12);
        return;
    }

    i2d_PKCS12_bio(bio, p12);

    PKCS12_free(p12);
    BIO_free(bio);
}

void MainWindow::saveCertificateCSR(X509 *cert, const QString &filePath) {
    BIO *bio = BIO_new_file(filePath.toUtf8().constData(), "w");
    if (!bio) return;

    PEM_write_bio_X509(bio, cert);
    BIO_free(bio);
}
