#ifndef ONBOARDINGDIALOG_H
#define ONBOARDINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>

class OnboardingDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnboardingDialog(QWidget* parent = nullptr);
    QString username() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onUsernameChanged(const QString& text);
    void checkAvailability();
    void onCheckFinished(QNetworkReply* reply);
    void onRegisterClicked();
    void onRegisterFinished(QNetworkReply* reply);

private:
    QLineEdit* m_usernameInput;
    QLabel* m_statusLabel;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QPushButton* m_continueBtn;
    QTimer* m_debounceTimer;
    QNetworkAccessManager* m_networkManager;
    QString m_username;
    bool m_isAvailable;
    bool m_isChecking;
};

#endif // ONBOARDINGDIALOG_H
