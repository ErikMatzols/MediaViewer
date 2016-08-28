#ifndef LISTENSERVERDIALOG_HPP
#define LISTENSERVERDIALOG_HPP

#include <QDialog>

class TcpCommand;
class MainWindow;
class QLabel;

class ListenServerDialog : public QDialog {
    Q_OBJECT

public:
    ListenServerDialog(MainWindow* parent);
    ~ListenServerDialog();

    void initializeServer(TcpCommand* tcpServer);

    bool running()
    {
        return mRunning;
    }

    int error()
    {
        return mError;
    }

protected:
private slots:
    void minimizeToTrayPressed();
    void cancelPressed();

private:
    TcpCommand* m_tcpServer;
    MainWindow* mMainWindow;
    int mError;
    QLabel* mLabel;
    bool mRunning;
};

#endif
