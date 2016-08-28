#include "ColorControlDialog.hpp"
#include "CustomSlider.hpp"
#include "Defines.hpp"
#include "MainWindow.hpp"
#include "VideoRenderer.hpp"
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

ColorControlDialog::ColorControlDialog(VideoRenderer* videoRenderer)
    : CustomWindow(0)
{
    mVideoRenderer = videoRenderer;

    setupUI();

    setTitle("Color Control");
    setWindowTitle("Color Control");
#ifndef NO_IMAGE_ALLOC
    setIcon("../resources/Images/title16.png");
    setWindowIcon(QIcon("../resources/Images/title16.png"));
#endif
}

ColorControlDialog::~ColorControlDialog()
{
}

void ColorControlDialog::setupUI()
{
    QLabel* label1 = new QLabel("Hue");
    QLabel* label2 = new QLabel("Brightness");
    QLabel* label3 = new QLabel("Contrast");
    QLabel* label4 = new QLabel("Saturation");

    QCheckBox* enableBox = new QCheckBox("Enabled", this);
    connect(enableBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));

    QPushButton* defaultButton = new QPushButton("Defaults", this);
    connect(defaultButton, SIGNAL(pressed()), this, SLOT(defaultButtonPressed()));

    mHueSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mHueSlider, SIGNAL(valueChanged(int)), this, SLOT(hueValueChanged(int)));
    mBrightSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mBrightSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessValueChanged(int)));
    mContrastSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mContrastSlider, SIGNAL(valueChanged(int)), this, SLOT(contrastValueChanged(int)));
    mSaturSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mSaturSlider, SIGNAL(valueChanged(int)), this, SLOT(saturValueChanged(int)));

    mHueSlider->setRange(-192, 192);
    mBrightSlider->setRange(-192, 192);
    mContrastSlider->setRange(-192, 192);
    mSaturSlider->setRange(-192, 192);

    mHueSlider->setEnabled(false);
    mBrightSlider->setEnabled(false);
    mContrastSlider->setEnabled(false);
    mSaturSlider->setEnabled(false);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(enableBox);
    hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(defaultButton);

    QGridLayout* gLayout = new QGridLayout;
    gLayout->addWidget(label1, 0, 0, 1, 1);
    gLayout->addWidget(mHueSlider, 0, 1, 1, 1);
    gLayout->addWidget(label2, 1, 0, 1, 1);
    gLayout->addWidget(mBrightSlider, 1, 1, 1, 1);
    gLayout->addWidget(label3, 2, 0, 1, 1);
    gLayout->addWidget(mContrastSlider, 2, 1, 1, 1);
    gLayout->addWidget(label4, 3, 0, 1, 1);
    gLayout->addWidget(mSaturSlider, 3, 1, 1, 1);

    QWidget* mainWidget = new QWidget;
    QVBoxLayout* vLayout = new QVBoxLayout(mainWidget);
    vLayout->addLayout(hLayout);
    vLayout->addLayout(gLayout);

    setMainWidget(mainWidget);

    QString sliderStyle = MainWindow::loadStyleSheet("../style/ControllerStyle.css");
    setStyleSheet(sliderStyle);
}

void ColorControlDialog::stateChanged(int state)
{
    if (state == Qt::Checked) {
        //mHueSlider->setEnabled(true);
        mBrightSlider->setEnabled(true);
        mContrastSlider->setEnabled(true);
        //mSaturSlider->setEnabled(true);
        mVideoRenderer->setColorFilterEnabled(true);
    } else {
        mHueSlider->setEnabled(false);
        mBrightSlider->setEnabled(false);
        mContrastSlider->setEnabled(false);
        mSaturSlider->setEnabled(false);
        mVideoRenderer->setColorFilterEnabled(false);
    }
}

void ColorControlDialog::hueValueChanged(int /*h*/)
{
    //mVideoRenderer->setHue(h);
}

void ColorControlDialog::contrastValueChanged(int c)
{
    mVideoRenderer->setContrast(c);
}

void ColorControlDialog::brightnessValueChanged(int b)
{
    mVideoRenderer->setBrightness(b);
}

void ColorControlDialog::saturValueChanged(int /*s*/)
{
    //mVideoRenderer->setSaturation(s);
}

void ColorControlDialog::defaultButtonPressed()
{
    mHueSlider->setValue(0);
    mBrightSlider->setValue(0);
    mContrastSlider->setValue(0);
    mSaturSlider->setValue(0);
}
