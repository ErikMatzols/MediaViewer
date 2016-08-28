#include "MenuWidget.hpp"
#include "CustomButton.hpp"
#include <QHBoxLayout>
#include <QMenuBar>

MenuWidget::MenuWidget(QWidget* parent, QMenuBar* menuBar, CustomButton* minButton, CustomButton* maxButton, CustomButton* closeButton)
    : QWidget(parent)
{
    QHBoxLayout* hLayout1 = new QHBoxLayout(this);
    menuBar->setMaximumWidth(menuBar->minimumWidth());
    hLayout1->addWidget(menuBar);
    hLayout1->addItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum));
    hLayout1->addWidget(minButton);
    hLayout1->addWidget(maxButton);
    hLayout1->addWidget(closeButton);
    hLayout1->setMargin(0);
    //int h = menuBar->minimumHeight();
    //int h2 = menuBar->height();
    setMaximumHeight(20);
}

MenuWidget::~MenuWidget()
{
}
