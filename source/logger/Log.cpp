#include "Log.hpp"
#include "Defines.hpp"
#include "Mainwindow.hpp"
#include <QApplication>
#include <QIcon>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>

void outcallback(const char* ptr, std::streamsize count, void* /*data*/)
{
    static QString str;
    (void)count;
    str.append(ptr);
    if (str.size() > 0 && str[str.size() - 1] == '\n') {
        //#ifdef WRITE_TO_CONSOLE
        printf("%s", str.toStdString().c_str());
        //#else
        //        LogEvent* event = new LogEvent(LogEvent::Info);
        //        event->setText(str);
        //        QApplication::postEvent(&Log::Instance(), event);
        //#endif
        str.clear();
    }
}

Log& Log::Instance()
{
    static Log instance(0);
    return instance;
}

Log::Log(QWidget* parent)
    : CustomWindow(parent)
{
    QWidget* mainWidget = new QWidget;

    mTextEdit = new QPlainTextEdit(mainWidget);
    //mTextEdit->setMaximumBlockCount(10);
    mTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    mTextEdit->setReadOnly(true);
    mTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mRedirector = new StdRedirector<>(std::cout, outcallback, NULL);

    QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);
    vLayout->addWidget(mTextEdit);
    vLayout->addSpacing(14);
    vLayout->setMargin(0);

    setTitle("Log");
    setWindowTitle("Log");
#ifndef NO_IMAGE_ALLOC
    setIcon("../resources/Images/title16.png");
    setWindowIcon(QIcon("../resources/Images/title16.png"));
#endif
    resize(360, 240);

    setMainWidget(mainWidget);
    setStyleSheet(MainWindow::loadStyleSheet("../style/LogStyle.css"));
}

Log::~Log()
{
    delete mRedirector;
}

void Log::customEvent(QEvent* event)
{
    LogEvent* logEvent = static_cast<LogEvent*>(event);
    if (logEvent->type() == LogEvent::Info)
        mTextEdit->insertPlainText(logEvent->logText());
    else if (logEvent->type() == LogEvent::Warning)
        mTextEdit->insertPlainText("Warning: " + logEvent->logText());
    else if (logEvent->type() == LogEvent::Error)
        mTextEdit->insertPlainText("Error: " + logEvent->logText());
    // scroll fix
    mTextEdit->verticalScrollBar()->setValue(mTextEdit->verticalScrollBar()->maximum());
}
