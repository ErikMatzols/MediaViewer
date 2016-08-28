#ifndef MENUWIDGET_HPP
#define MENUWIDGET_HPP

#include <QWidget>
class QMenuBar;
class CustomButton;

class MenuWidget : public QWidget {
    Q_OBJECT
public:
    MenuWidget(QWidget* parent, QMenuBar* menuBar, CustomButton* minButton, CustomButton* maxButton, CustomButton* closeButton);
    ~MenuWidget();

protected:
private:
};

#endif
