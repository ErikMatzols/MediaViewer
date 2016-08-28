#include "WebView.hpp"

WebView::WebView(MainWindow* parent)
   : QWebEngineView(parent)
{
   mParent = parent;
}

WebView::~WebView()
{
}

void WebView::mousePressEvent(QMouseEvent* event)
{
   QWebEngineView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
   QWebEngineView::mouseReleaseEvent(event);
}

void WebView::keyPressEvent(QKeyEvent* event)
{
   if (!mParent->mouseEmulation())
      QWebEngineView::keyPressEvent(event);
}

void WebView::keyReleaseEvent(QKeyEvent* event)
{
   if (!mParent->mouseEmulation())
      QWebEngineView::keyReleaseEvent(event);
}
