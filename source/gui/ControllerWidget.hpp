#ifndef CONTROLLERWIDGET_HPP
#define CONTROLLERWIDGET_HPP

#include <QWidget>
class QMenu;
class QActionGroup;
class MainWindow;
class CustomButton;
class CustomSlider;
class QLabel;
class QTimer;
class QPushButton;
class ChannelInfo;

enum ControllerMode { DEMUXER_CONTROL,
    TUNER_CONTROL,
    WEB_CONTROL };

class ControllerWidget : public QWidget {
    Q_OBJECT

public:
    ControllerWidget(MainWindow* parent);
    ~ControllerWidget();

    void setMode(ControllerMode mode);
    void setPaused(bool paused);
    bool isPaused();
    void updateChannelIndex(int index);
    void disableProgress();

public slots:
    void progressUpdate(double time);
    void registerDuration(int duration);
    void registerChannels(QList<ChannelInfo*> channels, int currentIndex);

    void volumePressed();
    void updateVolume(int volume);

    void volumeUp();
    void volumeDown();

protected:
    void setupUI(MainWindow* parent);

private slots:
    void mute();
    void unmute();
    void updateElapsedLabel(int elapsed);

    void popupChannelMenu();

    void channelMenuPressed(QAction* action);

private:
    QMenu* mChannelMenu;

    QActionGroup* mChannelActionGroup;
    CustomButton* mChannelButton;

    CustomButton* mPlayPauseButton;
    CustomButton* mStopButton;
    CustomButton* mSkipBButton;
    CustomButton* mSkipFButton;
    CustomButton* mSeekBButton;
    CustomButton* mSeekFButton;
    CustomButton* mVolumeButton;

    CustomSlider* mProgressSlider;
    CustomSlider* mVolumeSlider;

    QLabel* mElapsedLabel;
    QLabel* mDurationLabel;
    QTimer* mTimer;

    MainWindow* mParent;
    int mPrevVolume;
    bool mPaused;
    bool mVolumeMuted;
};

#endif
