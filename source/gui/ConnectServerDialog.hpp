#ifndef CONNECTSERVERDIALOG_HPP
#define CONNECTSERVERDIALOG_HPP

#include <QDialog>

class QTextEdit;
class QLineEdit;
class TcpCommand;

class ConnectServerDialog : public QDialog {
    Q_OBJECT
public:
    ConnectServerDialog(TcpCommand* tcpClient, QWidget* parent);
    ~ConnectServerDialog();

    QString getPassword();

protected:
private slots:
    void connectPressed();
    void cancelPressed();
    void connectSuccess();
    void connectError(const QString& msg);

private:
    QLineEdit* m_lineAddress;
    QLineEdit* mLinePort;
    QLineEdit* mLinePassword;
    QPushButton* connectButton;
    QPushButton* cancelButton;

    TcpCommand* m_tcpClient;
};

#endif
