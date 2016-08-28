#ifndef LOG_HPP
#define LOG_HPP

#include "CustomWindow.hpp"
#include "StdRedirector.hpp"
#include <QEvent>

class QPlainTextEdit;

class LogEvent : public QEvent {
public:
    enum LogType {
        Info = QEvent::User + 1,
        Warning,
        Error
    };

    LogEvent(LogEvent::LogType type)
        : QEvent(static_cast<QEvent::Type>(type))
    {
    }

    void setText(const QString& text) { this->text = text.toStdString(); }
    QString logText() { return QString(text.c_str()); }

private:
    std::string text;
};

class Log : public CustomWindow {
    Q_OBJECT

public:
    static Log& Instance();

protected:
    Log(QWidget* parent);
    ~Log();

    void customEvent(QEvent* event);

private:
    QPlainTextEdit* mTextEdit;
    StdRedirector<>* mRedirector;
};

#endif
