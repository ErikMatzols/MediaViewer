#include "CustomSlider.hpp"
#include <QMouseEvent>
#include <QToolTip>

CustomSlider::CustomSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
    mIgnoreUpdate = false;
    setPageStep(1);
    setSingleStep(1);
}

CustomSlider::~CustomSlider()
{
}

void CustomSlider::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        mIgnoreUpdate = true;
        float frac = e->x() / (double)width();
        int val = minimum() + (frac * (abs(minimum()) + maximum()) + 0.5f);

        setSliderPosition(val);

        e->accept();
    }
    QSlider::mousePressEvent(e);
    QWidget::mousePressEvent(e);
}

void CustomSlider::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        QSlider::mouseReleaseEvent(e);
        emit mouseReleased(value());
        mIgnoreUpdate = false;
    }
    QSlider::mouseReleaseEvent(e);
}

void CustomSlider::keyPressEvent(QKeyEvent* ev)
{
    ev->accept();
}

void CustomSlider::updateSlider(int value)
{
    if (!mIgnoreUpdate)
        setSliderPosition(value);
}
