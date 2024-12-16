#include "mainwindow.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QTabWidget *tabWidget = new QTabWidget(this);

    {
        QWidget *checkTab = new QWidget();
        QVBoxLayout *checkLayout = new QVBoxLayout(checkTab);

        QHBoxLayout *checkFileLayout = new QHBoxLayout();
        checkFileEdit = new QLineEdit();
        QPushButton *openCheckFileButton = new QPushButton("Открыть файл");
        checkFileLayout->addWidget(checkFileEdit);
        checkFileLayout->addWidget(openCheckFileButton);

        QHBoxLayout *checkSigLayout = new QHBoxLayout();
        checkSigEdit = new QLineEdit();
        QPushButton *openCheckSigButton = new QPushButton("Открыть подпись");
        checkSigLayout->addWidget(checkSigEdit);
        checkSigLayout->addWidget(openCheckSigButton);

        QHBoxLayout *publicKeyLayout = new QHBoxLayout();
        publicKeyEdit = new QLineEdit();
        QPushButton *openPublicKeyButton = new QPushButton("Открыть ключ");
        publicKeyLayout->addWidget(publicKeyEdit);
        publicKeyLayout->addWidget(openPublicKeyButton);

        checkButton = new QPushButton("Проверить");
        checkButton->setEnabled(false);

        checkLayout->addLayout(checkFileLayout);
        checkLayout->addLayout(checkSigLayout);
        checkLayout->addLayout(publicKeyLayout);
        checkLayout->addWidget(checkButton);

        checkTab->setLayout(checkLayout);

        tabWidget->addTab(checkTab, "Проверить");

        connect(openCheckFileButton, &QPushButton::clicked, this, &MainWindow::onOpenMainFileCheckClicked);
        connect(openCheckSigButton, &QPushButton::clicked, this, &MainWindow::onOpenSignatureFileCheckClicked);
        connect(openPublicKeyButton, &QPushButton::clicked, this, &MainWindow::onOpenPublicKeyClicked);
        connect(checkButton, &QPushButton::clicked, this, &MainWindow::onCheckSignatureClicked);
    }

    {
        QWidget *signTab = new QWidget();
        QVBoxLayout *signLayout = new QVBoxLayout(signTab);

        QHBoxLayout *signFileLayout = new QHBoxLayout();
        signFileEdit = new QLineEdit();
        QPushButton *openSignFileButton = new QPushButton("Открыть файл");
        signFileLayout->addWidget(signFileEdit);
        signFileLayout->addWidget(openSignFileButton);

        QHBoxLayout *privateKeyLayout = new QHBoxLayout();
        privateKeyEdit = new QLineEdit();
        QPushButton *openPrivateKeyButton = new QPushButton("Открыть ключ");
        privateKeyLayout->addWidget(privateKeyEdit);
        privateKeyLayout->addWidget(openPrivateKeyButton);

        signButton = new QPushButton("Подписать");
        signButton->setEnabled(false);

        signLayout->addLayout(signFileLayout);
        signLayout->addLayout(privateKeyLayout);
        signLayout->addWidget(signButton);

        signTab->setLayout(signLayout);

        tabWidget->addTab(signTab, "Подписать");

        connect(openSignFileButton, &QPushButton::clicked, this, &MainWindow::onOpenFileToSignClicked);
        connect(openPrivateKeyButton, &QPushButton::clicked, this, &MainWindow::onOpenPrivateKeyClicked);
        connect(signButton, &QPushButton::clicked, this, &MainWindow::onSignFileClicked);
    }

    setCentralWidget(tabWidget);
    setWindowTitle("Приложение для ЭЦП");
}

MainWindow::~MainWindow() {}

void MainWindow::onOpenMainFileCheckClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл для проверки");
    if (!fileName.isEmpty()) {
        checkFileEdit->setText(fileName);
        updateCheckTabButtonState();
    }
}

void MainWindow::onOpenSignatureFileCheckClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл подписи");
    if (!fileName.isEmpty()) {
        checkSigEdit->setText(fileName);
        updateCheckTabButtonState();
    }
}

void MainWindow::onOpenPublicKeyClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите публичный ключ");
    if (!fileName.isEmpty()) {
        publicKeyEdit->setText(fileName);
        updateCheckTabButtonState();
    }
}

void MainWindow::onCheckSignatureClicked() {
    QString filePath = checkFileEdit->text().trimmed();
    QString sigPath = checkSigEdit->text().trimmed();
    QString pubKeyPath = publicKeyEdit->text().trimmed();

    if (verifySignature(filePath, sigPath, pubKeyPath)) {
        QMessageBox::information(this, "Результат", "Подпись действительна.");
    } else {
        QMessageBox::warning(this, "Результат", "Подпись недействительна.");
    }
}

void MainWindow::updateCheckTabButtonState() {
    bool enable = !checkFileEdit->text().isEmpty() &&
                  !checkSigEdit->text().isEmpty() &&
                  !publicKeyEdit->text().isEmpty();
    checkButton->setEnabled(enable);
}

void MainWindow::onOpenFileToSignClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл для подписания");
    if (!fileName.isEmpty()) {
        signFileEdit->setText(fileName);
        updateSignTabButtonState();
    }
}

void MainWindow::onOpenPrivateKeyClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите закрытый ключ");
    if (!fileName.isEmpty()) {
        privateKeyEdit->setText(fileName);
        updateSignTabButtonState();
    }
}

void MainWindow::onSignFileClicked() {
    QString filePath = signFileEdit->text().trimmed();
    QString privKeyPath = privateKeyEdit->text().trimmed();

    QString sigPath;
    if (signFile(filePath, privKeyPath, sigPath)) {
        QMessageBox::information(this, "Результат", QString("Подпись успешно создана: %1").arg(sigPath));
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось подписать файл.");
    }
}

void MainWindow::updateSignTabButtonState() {
    bool enable = !signFileEdit->text().isEmpty() &&
                  !privateKeyEdit->text().isEmpty();
    signButton->setEnabled(enable);
}

bool MainWindow::signFile(const QString &filePath, const QString &privateKeyPath, QString &signaturePath) {
    Q_UNUSED(privateKeyPath);
    signaturePath = filePath + ".sig";
    QFile signatureFile(signaturePath);
    if (signatureFile.open(QIODevice::WriteOnly)) {
        signatureFile.write("FakeSignatureData");
        signatureFile.close();
        return true;
    }
    return false;
}

bool MainWindow::verifySignature(const QString &filePath, const QString &signaturePath, const QString &publicKeyPath) {
    Q_UNUSED(publicKeyPath);
    QFile sigFile(signaturePath);
    if (sigFile.open(QIODevice::ReadOnly)) {
        QByteArray data = sigFile.readAll();
        sigFile.close();
        return data == "FakeSignatureData";
    }
    return false;
}
