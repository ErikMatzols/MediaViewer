#ifndef CUSTOMWINDOW_HPP
#define CUSTOMWINDOW_HPP

#include <QApplication>
#include <QBitmap>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRegion>
#include <QSizeGrip>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

#include <Windows.h>
#include <iostream>

#include "CustomButton.hpp"

class CustomWindow : public QWidget {
    Q_OBJECT

public:
    CustomWindow(QWidget* parent = 0, bool modal = false)
        : QWidget(parent, Qt::FramelessWindowHint
                  | Qt::CustomizeWindowHint
                  | Qt::WindowSystemMenuHint)
    {
        LONG style = GetWindowLong((HWND)winId(), GWL_STYLE);
        SetWindowLong((HWND)winId(), GWL_STYLE, style | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        mMaximized = false;

        mIcon = new QLabel(this);
        mIcon->setVisible(false);
        mIconSet = false;

        mTitle = new QLabel("", this);
        mTitle->setVisible(false);
        mTitleSet = false;

        mMenuBar = new QMenuBar(this);
        mMenuBar->setVisible(false);
        mMenuBarSet = false;

        mMinButton = new CustomButton(this,
            "../resources/Images/minimize.png",
            "../resources/Images/minimize_hov.png",
            "../resources/Images/minimize.png",
            "Minimize");
        connect(mMinButton, SIGNAL(buttonPressed()), this, SLOT(showMinimized()));

        mMaxButton = new CustomButton(this, "../resources/Images/maximize.png",
            "../resources/Images/maximize_hov.png",
            "../resources/Images/maximize.png",
            "Maximize");
        connect(mMaxButton, SIGNAL(buttonPressed()), this, SLOT(toggleMaximized()));

        mCloseButton = new CustomButton(this,
            "../resources/Images/close.png",
            "../resources/Images/close_hov.png",
            "../resources/Images/close.png",
            "Exit MediaViewer");
        connect(mCloseButton, SIGNAL(buttonPressed()), this, SLOT(close()));

        QHBoxLayout* menuLayout = new QHBoxLayout;
        menuLayout->addWidget(mIcon);
        menuLayout->addSpacing(4);
        menuLayout->addWidget(mTitle);
        menuLayout->addWidget(mMenuBar);
        menuLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        menuLayout->addWidget(mMinButton);
        menuLayout->addSpacing(4);
        menuLayout->addWidget(mMaxButton);
        menuLayout->addSpacing(4);
        menuLayout->addWidget(mCloseButton);

        mWindowLayout = new QVBoxLayout(this);
        mWindowLayout->addLayout(menuLayout);

        int left, top, right, bottom;
        mWindowLayout->getContentsMargins(&left, &top, &right, &bottom);
        mWindowLayout->setContentsMargins(8, 4, 8, 4);
        mWindowLayout->setSpacing(0);
        mSizeGrip = new QSizeGrip(this);

        if (modal)
            setWindowModality(Qt::ApplicationModal);

        //setAttribute(Qt::WA_NoSystemBackground, true);
        //setAttribute(Qt::WA_TranslucentBackground, true);
        //setAttribute(Qt::WA_PaintOnScreen);
    }

    ~CustomWindow()
    {
    }

    void getContentsMargins(int& left, int& top, int& right, int& bottom)
    {
        mWindowLayout->getContentsMargins(&left, &top, &right, &bottom);
    }

    void setMainWidget(QWidget* widget)
    {
        mWindowLayout->addWidget(widget);
    }

    QMenuBar* getMenuBar()
    {
        mMenuBarSet = true;
        mMenuBar->setVisible(true);
        return mMenuBar;
    }

    void setMenuWidget(QWidget* menu)
    {
        mMenuLayout->insertWidget(2, menu);
    }

    void setTitle(const QString& title)
    {
        mTitleSet = true;
        mTitle->setText(title);
        mTitle->setVisible(true);
    }

    void setIcon(const QString& icon)
    {
        mIconSet = true;
        mIcon->setPixmap(QPixmap(icon));
        mIcon->setVisible(true);
    }

signals:
    void minimizePressed();
    void maximizePressed();
    void closePressed();

public slots:
    void showNormal()
    {
        mMaxButton->setImage("../resources/Images/maximize.png",
            "../resources/Images/maximize_hov.png",
            "../resources/Images/maximize.png",
            "Maximize");
        mSizeGrip->setVisible(true);

        if (mIconSet)
            mIcon->setVisible(true);
        if (mTitleSet)
            mTitle->setVisible(true);
        if (mMenuBarSet)
            mMenuBar->setVisible(true);

        mMinButton->setVisible(true);
        mMaxButton->setVisible(true);
        mCloseButton->setVisible(true);
        mWindowLayout->setContentsMargins(8, 4, 8, 4);
        mMaximized = false;

        move(mPrevPos);
        resize(mPrevSize);
    }

    void showMaximized()
    {
        mMaxButton->setImage("../resources/Images/normalize.png",
            "../resources/Images/normalize_hov.png",
            "../resources/Images/normalize.png",
            "Normalize");
        mSizeGrip->setVisible(false);
        mMaximized = true;
        mPrevPos = pos();
        mPrevSize = size();
        QRect rect = QApplication::desktop()->availableGeometry(this);
        move(0, 0);
        resize(rect.width(), rect.height());
    }

    void showFullScreen()
    {
        mWindowLayout->setContentsMargins(0, 0, 0, 0);
        mIcon->setVisible(false);
        mTitle->setVisible(false);
        mMenuBar->setVisible(false);
        mMinButton->setVisible(false);
        mMaxButton->setVisible(false);
        mCloseButton->setVisible(false);
        mPrevPos = pos();
        mPrevSize = size();
        QWidget::showFullScreen();
        clearMask();
    }

    bool isMaximized()
    {
        return mMaximized;
    }

protected:
    void mousePressEvent(QMouseEvent* event)
    {
        if (!isFullScreen()) {
            mPrevMousePos = event->globalPos();
        }
    }

    void mouseReleaseEvent(QMouseEvent* /*event*/)
    {
        if (!isFullScreen()) {
            resize(width(), height());
        }
    }

    void mouseMoveEvent(QMouseEvent* event)
    {
        if (!isFullScreen() && mWindowState != WindowState::RESIZING) {
            mWindowState = WindowState::MOVING;
            move(pos() + (event->globalPos() - mPrevMousePos));
            mPrevMousePos = event->globalPos();
        } else if (mWindowState == WindowState::RESIZING) {
            mWindowState = WindowState::MOVING;
        }
    }

    void paintEvent(QPaintEvent* /*event*/)
    {
        if (!isFullScreen()) {
            QPainter painter(this);

            QPen pen(QColor(3, 9, 14));
            QPen pen2(QColor(109, 114, 119));
            pen2.setWidth(1);
            painter.setPen(pen);
            painter.setBrush(QColor(40, 40, 40));
            painter.drawRoundedRect(0, 0, width() - 1, height() - 1, 8, 8);

            painter.setPen(pen2);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(1, 1, width() - 3, height() - 3, 8, 8);
        }
    }

    void resizeEvent(QResizeEvent* /*event*/)
    {
        if (!isFullScreen()) {
            mWindowState = WindowState::RESIZING;
            QImage mask_img(width(), height(), QImage::Format_Mono);
            mask_img.fill(0xff);
            QPainter mask_ptr(&mask_img);
            mask_ptr.setRenderHint(QPainter::HighQualityAntialiasing);
            mask_ptr.setPen(Qt::NoPen);
            mask_ptr.setBrush(QBrush(QColor(0, 0, 0)));
            mask_ptr.drawRoundedRect(QRectF(0, 0, width() - 1, height() - 1), 8, 8);
            QBitmap bmp = QBitmap::fromImage(mask_img);
            setMask(bmp);

            mSizeGrip->move(width() - 22, height() - 23);
            mSizeGrip->resize(20, 20);
        }
    }

private slots:
    void toggleMaximized()
    {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    }

private:
    QPoint mPrevMousePos;
    QPoint mPrevPos;
    QSize mPrevSize;
    bool mMaximized;
    bool mIconSet;
    bool mTitleSet;
    bool mMenuBarSet;

    QLabel* mIcon;
    QLabel* mTitle;
    QMenuBar* mMenuBar;
    CustomButton* mMinButton;
    CustomButton* mMaxButton;
    CustomButton* mCloseButton;

    QHBoxLayout* mMenuLayout;
    QVBoxLayout* mWindowLayout;
    QSizeGrip* mSizeGrip;

    enum class WindowState {
        MOVING,
        RESIZING
    } mWindowState
        = WindowState::MOVING;
};

#endif
