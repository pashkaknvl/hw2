#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::on_lineEdit_returnPressed);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_pushButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Открыть Сертификат"), "", tr("Файлы сертификата (*.pem *.crt *.cer *.p7b *.pfx *.p12 *.csr)"));
    if (!filename.isEmpty()) processCertificate(filename);
}

void MainWindow::on_lineEdit_returnPressed() {
    QString filename = ui->lineEdit->text().trimmed();
    if (!QFile::exists(filename)) {
        ui->textEdit->append("Файл не существует: " + filename);
        return;
    }
    processCertificate(filename);
}

void MainWindow::processCertificate(const QString& filename) {
    ui->textEdit->clear();
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        ui->textEdit->append("Ошибка открытия файла: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(fileData.data(), fileData.size());
    if (!bio) {
        ui->textEdit->append("Ошибка при создании BIO.");
        return;
    }

    X509 *x509Cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    if (!x509Cert) {
        BIO_reset(bio);
        x509Cert = d2i_X509_bio(bio, nullptr);
    }

    BIO_free(bio);

    if (x509Cert) {
        ui->textEdit->append(getCertificateInfo(x509Cert));
        X509_free(x509Cert);
    } else {
        ui->textEdit->append("Ошибка при разборе сертификата! Проверьте файл.");
        ERR_print_errors_fp(stderr);
    }

    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}

QString MainWindow::getCertificateInfo(X509 *cert) {
    QString info;
    char *subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    char *issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);

    info += "Субъект:\n" + QString::fromLatin1(subject) + "\n";
    info += "Издатель:\n" + QString::fromLatin1(issuer) + "\n";

    OPENSSL_free(subject);
    OPENSSL_free(issuer);

    return info;
}
