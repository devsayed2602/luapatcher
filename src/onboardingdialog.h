#ifndef ONBOARDINGDIALOG_H
#define ONBOARDINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>

class OnboardingDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnboardingDialog(QWidget* parent = nullptr);
    QString username() const;
    QJsonObject userData() const;
    bool isGuest() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onUsernameChanged(const QString& text);
    void checkAvailability();
    void onCheckFinished(QNetworkReply* reply);
    void onPrimaryClicked();
    void onAuthFinished(QNetworkReply* reply);
    void switchMode(int mode);
    void onGuestClicked();

private:
    enum Mode { WELCOME, LOGIN, REGISTER };
    int m_currentMode;

    QWidget* m_welcomeView;
    QWidget* m_formView;

    QLineEdit* m_usernameInput;
    QLineEdit* m_passwordInput;
    QLabel* m_statusLabel;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QPushButton* m_continueBtn;
    QPushButton* m_backBtn;

    QTimer* m_debounceTimer;
    QNetworkAccessManager* m_networkManager;
    QString m_username;
    QJsonObject m_userData;
    bool m_isAvailable;
    bool m_isChecking;
    bool m_isGuest;
};

#endif // ONBOARDINGDIALOG_H
