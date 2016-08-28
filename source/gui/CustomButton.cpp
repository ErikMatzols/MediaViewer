#include "CustomButton.hpp"
#include "Defines.hpp"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmapCache>
#include <iostream>

CustomButton::CustomButton(QWidget* parent, const QString& unp, const QString& hov, const QString& pre, const QString& tooltip)
    : QWidget(parent)
{
#ifndef NO_IMAGE_ALLOC
    if (!QPixmapCache::find(unp, &mPix[0])) {
        mPix[0].load(unp);
        QPixmapCache::insert(unp, mPix[0]);
    }
    if (!QPixmapCache::find(hov, &mPix[1])) {
        mPix[1].load(hov);
        QPixmapCache::insert(hov, mPix[1]);
    }
    if (!QPixmapCache::find(pre, &mPix[2])) {
        mPix[2].load(pre);
        QPixmapCache::insert(pre, mPix[2]);
    }
#endif
    mPainter = new QPainter;
    setMinimumSize(mPix[0].width(), mPix[0].height());
    setMaximumSize(mPix[0].width(), mPix[0].height());
    mLabelMode = false;
    mState = NORMAL;
    setToolTip(tooltip);
}

CustomButton::CustomButton(QWidget* parent, const QString& label, int w, int h, const QString& colUnp, const QString& colHov, const QString& tooltip)
   : QWidget(parent)
{
    mPainter = new QPainter;
    mLabel = label;
    mLabelMode = true;
    mLabelColor = colUnp;
    mColUnp = colUnp;
    mColHov = colHov;
    setMinimumSize(w, h);
    setMaximumSize(w, h);
    mState = NORMAL;
    setToolTip(tooltip);
}

CustomButton::~CustomButton()
{
    delete mPainter;
}

void CustomButton::setImage(const QString& unp, const QString& hov, const QString& pre, const QString& tooltip)
{
#ifndef NO_IMAGE_ALLOC
    mPix[0].detach();
    mPix[1].detach();
    mPix[2].detach();
    if (!QPixmapCache::find(unp, &mPix[0])) {
        mPix[0].load(unp);
        QPixmapCache::insert(unp, mPix[0]);
    }
    if (!QPixmapCache::find(hov, &mPix[1])) {
        mPix[1].load(hov);
        QPixmapCache::insert(hov, mPix[1]);
    }
    if (!QPixmapCache::find(pre, &mPix[2])) {
        mPix[2].load(pre);
        QPixmapCache::insert(pre, mPix[2]);
    }
#endif
    setToolTip(tooltip);
    update();
}

void CustomButton::setLabel(const QString& label)
{
    mLabel = label;
    update();
}

void CustomButton::enterEvent(QEvent* /*event*/)
{
    mState = HOVER;
    if (mLabelMode) {
        mLabelColor = mColHov;
        setCursor(Qt::PointingHandCursor);
    }
    update();
}

void CustomButton::leaveEvent(QEvent* /*event*/)
{
    mState = NORMAL;
    if (mLabelMode) {
        mLabelColor = mColUnp;
        setCursor(Qt::ArrowCursor);
    }
    update();
}

void CustomButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        mState = PRESSED;
        update();
    }
    QWidget::mousePressEvent(event);
}

void CustomButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        mState = HOVER;
        emit buttonPressed();
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void CustomButton::paintEvent(QPaintEvent* /*event*/)
{
    mPainter->begin(this);
    if (mLabelMode) {
        mPainter->setPen(QColor(mLabelColor));
        mPainter->setFont(QFont("Arial", 10));
        mPainter->drawText(rect(), Qt::AlignVCenter | Qt::AlignLeft, mLabel);
    } else {
        mPainter->drawPixmap(0, 0, mPix[mState]);
    }
    mPainter->end();
}
