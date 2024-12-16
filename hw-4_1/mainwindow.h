#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;
class QPushButton;

    class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenMainFileCheckClicked();
    void onOpenSignatureFileCheckClicked();
    void onOpenPublicKeyClicked();
    void onCheckSignatureClicked();

    void onOpenFileToSignClicked();
    void onOpenPrivateKeyClicked();
    void onSignFileClicked();

private:
    QLineEdit *checkFileEdit;
    QLineEdit *checkSigEdit;
    QLineEdit *publicKeyEdit;
    QPushButton *checkButton;

    QLineEdit *signFileEdit;
    QLineEdit *privateKeyEdit;
    QPushButton *signButton;

    void updateCheckTabButtonState();
    void updateSignTabButtonState();

    bool signFile(const QString &filePath, const QString &privateKeyPath, QString &signaturePath);
    bool verifySignature(const QString &filePath, const QString &signaturePath, const QString &publicKeyPath);
};

#endif // MAINWINDOW_H
