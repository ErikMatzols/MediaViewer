#ifndef CUSTOMSLIDER_HPP
#define CUSTOMSLIDER_HPP

#include <QSlider>

class CustomSlider : public QSlider {
    Q_OBJECT

public:
    CustomSlider(Qt::Orientation orientation, QWidget* parent = 0);
    ~CustomSlider();

    void updateSlider(int value);

signals:
    void mouseReleased(int value);

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* ev);

private:
    bool mIgnoreUpdate;
};

#endif
