#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include "MainWindow.hpp"
#include <QWebEngineView>

class WebView : public QWebEngineView {
    Q_OBJECT

public:
    WebView(MainWindow* parent);

    virtual ~WebView();

protected:
    void mousePressEvent(QMouseEvent* event);

    void mouseReleaseEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent* event);

    void keyReleaseEvent(QKeyEvent* event);

private:
    MainWindow* mParent;
};

#endif
