#ifndef COLORCONTROLDIALOG_HPP
#define COLORCONTROLDIALOG_HPP

#include "CustomWindow.hpp"
class VideoRenderer;
class CustomSlider;

class ColorControlDialog : public CustomWindow {
    Q_OBJECT
public:
    ColorControlDialog(VideoRenderer* videoRenderer);
    ~ColorControlDialog();

protected:
    void setupUI();

private slots:
    void stateChanged(int);

    void hueValueChanged(int);
    void contrastValueChanged(int);
    void brightnessValueChanged(int);
    void saturValueChanged(int);

    void defaultButtonPressed();

private:
    VideoRenderer* mVideoRenderer;
    CustomSlider* mHueSlider;
    CustomSlider* mBrightSlider;
    CustomSlider* mContrastSlider;
    CustomSlider* mSaturSlider;
};

#endif
