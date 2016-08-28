#ifndef CUSTOMBUTTON_HPP
#define CUSTOMBUTTON_HPP

#include <QPixmap>
#include <QString>
#include <QWidget>

class QMouseEvent;

class CustomButton : public QWidget {
    Q_OBJECT

public:
    CustomButton(QWidget* parent, const QString& unp,
        const QString& hov, const QString& pre,
        const QString& tooltip);
    CustomButton(QWidget* parent, const QString& label, int w, int h, const QString& colUnp, const QString& colHov, const QString& tooltip);
    ~CustomButton();

    void setImage(const QString& unp, const QString& hov, const QString& pre, const QString& tooltip);
    void setLabel(const QString& label);

signals:
    void buttonPressed();

protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    enum ButtonState { NORMAL, HOVER, PRESSED };
    QString mLabel;
    QString mLabelColor;
    QString mColHov;
    QString mColUnp;
    bool mLabelMode;
    QPainter* mPainter;
    QPixmap mPix[3];
    ButtonState mState;
};

#endif
