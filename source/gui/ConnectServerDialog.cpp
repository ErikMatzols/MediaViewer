#include "ConnectServerDialog.hpp"
#include "TcpCommand.hpp"
#include <QApplication>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTime>

ConnectServerDialog::ConnectServerDialog(TcpCommand* tcpClient, QWidget* parent)
    : QDialog(parent)
    , m_tcpClient(tcpClient)
{
    connect(m_tcpClient, SIGNAL(tcpConnected()), this, SLOT(connectSuccess()));
    connect(m_tcpClient, SIGNAL(tcpError(const QString&)), this, SLOT(connectError(const QString&)));
    m_lineAddress = new QLineEdit(this);
    mLinePort = new QLineEdit(this);
    mLinePort->setValidator(new QIntValidator(0, 65535, this));
    mLinePassword = new QLineEdit(this);
    mLinePassword->setEchoMode(QLineEdit::Password);
    connectButton = new QPushButton("Connect", this);
    cancelButton = new QPushButton("Cancel", this);
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    QLabel* label1 = new QLabel("Address:", this);
    QLabel* label2 = new QLabel("Port:", this);
    QLabel* label3 = new QLabel("Password:", this);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(label1, 0, 0, 1, 1);
    gridLayout->addWidget(m_lineAddress, 1, 0, 1, 1);
    gridLayout->addWidget(label2, 0, 1, 1, 1);
    gridLayout->addWidget(mLinePort, 1, 1, 1, 1);
    gridLayout->addWidget(label3, 2, 0, 1, 1);
    gridLayout->addWidget(mLinePassword, 3, 0, 1, 1);

    QHBoxLayout* layoutHor = new QHBoxLayout;
    layoutHor->addWidget(connectButton);
    layoutHor->addWidget(cancelButton);
    gridLayout->addLayout(layoutHor, 4, 1, 1, 1);

    setWindowTitle("Connect To Server");
}

ConnectServerDialog::~ConnectServerDialog()
{
}

void ConnectServerDialog::connectPressed()
{
    QString ip = m_lineAddress->text();
    int port = mLinePort->text().toInt();
    m_tcpClient->connectTo(ip, port, 3000);
    connectButton->setEnabled(false);
}

void ConnectServerDialog::cancelPressed()
{
    m_tcpClient->disconnect();
    reject();
}

void ConnectServerDialog::connectSuccess()
{
    accept();
}

void ConnectServerDialog::connectError(const QString& /*msg*/)
{
    connectButton->setEnabled(true);
}

QString ConnectServerDialog::getPassword()
{
    return mLinePassword->text();
}
