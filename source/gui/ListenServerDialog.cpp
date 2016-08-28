#include "ListenServerDialog.hpp"
#include "Defines.hpp"
#include "MainWindow.hpp"
#include "TcpCommand.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ListenServerDialog::ListenServerDialog(MainWindow* parent)
    : QDialog(parent)
{
    mMainWindow = parent;
    mError = 0;
    mRunning = false;
    mLabel = new QLabel("", this);

    QPushButton* cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    QPushButton* minimizeToTrayButton = new QPushButton("Minimize To Tray", this);
    connect(minimizeToTrayButton, SIGNAL(clicked()), this, SLOT(minimizeToTrayPressed()));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(mLabel);
    layout->addWidget(minimizeToTrayButton);
    layout->addWidget(cancelButton);
#ifndef NO_IMAGE_ALLOC
    setWindowIcon(QIcon("../resources/Images/title24.png"));
#endif
    setWindowTitle("Listen Server");
}

ListenServerDialog::~ListenServerDialog()
{
}

void ListenServerDialog::initializeServer(TcpCommand* tcpServer)
{
    m_tcpServer = tcpServer;
    if (!m_tcpServer->listen()) {
        mError = 1;
        mLabel->setText("Error starting server");
    } else {
        mLabel->setText("Server running on port " + QString::number(m_tcpServer->getPort()));
        mRunning = true;
    }
}

void ListenServerDialog::cancelPressed()
{
    reject();
    m_tcpServer->disconnect();
    mRunning = false;
}

void ListenServerDialog::minimizeToTrayPressed()
{
    mMainWindow->minimizeToTray();
}
