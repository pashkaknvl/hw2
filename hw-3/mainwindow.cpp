#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // Layout для ввода сертификата
    QHBoxLayout *certificateLayout = new QHBoxLayout();
    QLabel *certificateLabel = new QLabel("Файл сертификата:", this);
    certificateLineEdit = new QLineEdit(this);
    openCertificateButton = new QPushButton("Открыть", this);

    certificateLayout->addWidget(certificateLabel);
    certificateLayout->addWidget(certificateLineEdit);
    certificateLayout->addWidget(openCertificateButton);

    mainLayout->addLayout(certificateLayout);

    // Layout для ввода запроса
    QHBoxLayout *requestLayout = new QHBoxLayout();
    QLabel *requestLabel = new QLabel("Файл запроса:", this);
    requestLineEdit = new QLineEdit(this);
    openRequestButton = new QPushButton("открыть", this); // Новая кнопка для выбора файла запроса

    requestLayout->addWidget(requestLabel);
    requestLayout->addWidget(requestLineEdit);
    requestLayout->addWidget(openRequestButton);

    mainLayout->addLayout(requestLayout);

    // Кнопка для подписи
    signButton = new QPushButton("Подпись запроса", this);
    signButton->setEnabled(false); // Кнопка отключена по умолчанию
    mainLayout->addWidget(signButton);

    connect(openCertificateButton, &QPushButton::clicked, this, &MainWindow::openCertificate);
    connect(openRequestButton, &QPushButton::clicked, this, &MainWindow::openRequest);
    connect(signButton, &QPushButton::clicked, this, &MainWindow::signCertificate);
}

MainWindow::~MainWindow() {}

void MainWindow::openCertificate() {
    QString certFilePath = QFileDialog::getOpenFileName(this, tr("Открыть сертификат"), "", tr("Файлы сертификатов (*.pem *.crt)"));
    if (certFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Еррор"), tr("Файл не выбран."));
        return;
    }

    // Загружаем сертификат для проверки
    X509 *cert = loadCertificate(certFilePath);
    if (!cert) {
        QMessageBox::critical(this, tr("Еррор"), tr("Ошибка загрузки сертификата, проверьте формат файла."));
        return;
    }

    certificateLineEdit->setText(certFilePath);
    signButton->setEnabled(true);
    X509_free(cert);
}

void MainWindow::openRequest() {
    QString requestFilePath = QFileDialog::getOpenFileName(this, tr("Открыть запрос сертификата"), "", tr("Файл запроса (*.csr *.pem)"));
    if (requestFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Еррор"), tr("Файл не выбран."));
        return;
    }

    requestLineEdit->setText(requestFilePath);
}

void MainWindow::signCertificate() {
    QString requestFilePath = requestLineEdit->text().trimmed();
    QString certFilePath = certificateLineEdit->text().trimmed();

    if (requestFilePath.isEmpty() || certFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Еррор"), tr("Необходимо указать оба файла - сертификата и запроса."));
        return;
    }

    if (!QFile::exists(requestFilePath) || !QFile::exists(certFilePath)) {
        QMessageBox::critical(this, tr("Oshibka"), tr("Неверные пути к файлам. Убедитесь, что файлы существуют."));
        return;
    }

    QString privateKeyPath = QFileDialog::getOpenFileName(this, tr("Открыть закрытый ключ"), "", tr("Файл ключа (*.pem *.key *.crt);;All Files (*)"));
    if (privateKeyPath.isEmpty()) {
        QMessageBox::warning(this, tr("ОШибка"), tr("Закрытый ключ не выбран."));
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, tr("Сохранить подпись сертификата"), "", tr("Файл сертификата (*.pem)"));
    if (savePath.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не указан путь к файлу для сохранения подписанного сертификата."));
        return;
    }

    X509 *signerCert = loadCertificate(certFilePath);
    EVP_PKEY *privateKey = loadPrivateKey(privateKeyPath);

    if (!signerCert || !privateKey) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось загрузить сертификат или закрытый ключ."));
        return;
    }

    if (signCertificateRequest(requestFilePath, savePath, signerCert, privateKey)) {
        QMessageBox::information(this, tr("Успех"), tr("Запрос на сертификат был успешно подписан и сохранен в:\n%1").arg(savePath));
    } else {
        QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка подписи сертификата запроса."));
    }

    X509_free(signerCert);
    EVP_PKEY_free(privateKey);
}

X509* MainWindow::loadCertificate(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return nullptr;

    QByteArray certData = file.readAll();
    BIO *bio = BIO_new_mem_buf(certData.data(), certData.size());
    X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return cert;
}

EVP_PKEY* MainWindow::loadPrivateKey(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return nullptr;

    QByteArray keyData = file.readAll();
    BIO *bio = BIO_new_mem_buf(keyData.data(), keyData.size());
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return pkey;
}

bool MainWindow::signCertificateRequest(const QString &requestFilePath, const QString &signedFilePath, X509 *signerCert, EVP_PKEY *privateKey) {
    QFile reqFile(requestFilePath);
    if (!reqFile.open(QIODevice::ReadOnly)) return false;

    QByteArray reqData = reqFile.readAll();
    BIO *reqBio = BIO_new_mem_buf(reqData.data(), reqData.size());
    X509_REQ *req = PEM_read_bio_X509_REQ(reqBio, nullptr, nullptr, nullptr);
    BIO_free(reqBio);
    reqFile.close();

    if (!req) return false;

    X509 *signedCert = X509_new();
    if (!signedCert) {
        X509_REQ_free(req);
        return false;
    }

    ASN1_INTEGER_set(X509_get_serialNumber(signedCert), 1);
    X509_set_issuer_name(signedCert, X509_get_subject_name(signerCert));
    X509_set_subject_name(signedCert, X509_REQ_get_subject_name(req));
    X509_set_pubkey(signedCert, X509_REQ_get_pubkey(req));
    X509_gmtime_adj(X509_get_notBefore(signedCert), 0);
    X509_gmtime_adj(X509_get_notAfter(signedCert), 31536000L);

    if (!X509_sign(signedCert, privateKey, EVP_sha256())) {
        X509_free(signedCert);
        X509_REQ_free(req);
        return false;
    }

    BIO *outBio = BIO_new_file(signedFilePath.toUtf8().data(), "w");
    if (!outBio) {
        X509_free(signedCert);
        X509_REQ_free(req);
        return false;
    }

    PEM_write_bio_X509(outBio, signedCert);
    BIO_free(outBio);
    X509_free(signedCert);
    X509_REQ_free(req);
    return true;
}
