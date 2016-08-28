#include "ControllerWidget.hpp"
#include "CustomButton.hpp"
#include "CustomSlider.hpp"
#include "MainWindow.hpp"
#include "WinTuner.hpp"
#include <QActionGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

ControllerWidget::ControllerWidget(MainWindow* parent)
    : QWidget(parent)
{
    mParent = parent;
    mPaused = false;
    mVolumeMuted = false;
    mPrevVolume = 100;
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), mParent, SLOT(checkCursorHidden()));
    setupUI(parent);
}

ControllerWidget::~ControllerWidget()
{
}

void ControllerWidget::setMode(ControllerMode mode)
{
    if (mode == DEMUXER_CONTROL) {
        mTimer->stop();
        mChannelButton->setVisible(false);
        mSeekBButton->setVisible(true);
        mSeekFButton->setVisible(true);
        mSkipBButton->setImage("../resources/Images/skipback.png", "../resources/Images/skipback_hov.png", "../resources/Images/skipback.png", "Skip Back");
        mSkipFButton->setImage("../resources/Images/skipforward.png", "../resources/Images/skipforward_hov.png", "../resources/Images/skipforward.png", "Skip Forward");
    } else if (mode == TUNER_CONTROL) {
        mChannelButton->setVisible(true);
        //mFilterButton->setVisible(true);
        mSeekBButton->setVisible(false);
        mSeekFButton->setVisible(false);
        mSkipBButton->setImage("../resources/Images/prevchannel.png", "../resources/Images/prevchannel_hov.png", "../resources/Images/prevchannel.png", "Prev Channel");
        mSkipFButton->setImage("../resources/Images/nextchannel.png", "../resources/Images/nextchannel_hov.png", "../resources/Images/nextchannel.png", "Next Channel");
        mParent->resetCursorHidden();
        mTimer->start(500);
    }
}

void ControllerWidget::setPaused(bool paused)
{
    mPaused = paused;
    if (paused) {
        mPlayPauseButton->setImage("../resources/Images/play.png", "../resources/Images/play_hov.png", "../resources/Images/play.png", "Play");
    } else {
        mPlayPauseButton->setImage("../resources/Images/pause.png", "../resources/Images/pause_hov.png", "../resources/Images/pause.png", "Pause");
    }
}

bool ControllerWidget::isPaused()
{
    return mPaused;
}

void ControllerWidget::disableProgress()
{
    mPlayPauseButton->setImage("../resources/Images/play.png", "../resources/Images/play_hov.png", "../resources/Images/play.png", "Play");
    mProgressSlider->updateSlider(0);
    mProgressSlider->setEnabled(false);
    mElapsedLabel->setText("00:00:00");
    mElapsedLabel->setEnabled(false);
    mDurationLabel->setText("00:00:00");
    mDurationLabel->setEnabled(false);
}

void ControllerWidget::progressUpdate(double time)
{
    mParent->checkCursorHidden();
    mProgressSlider->updateSlider(time);
}

void ControllerWidget::registerDuration(int duration)
{
    mElapsedLabel->setEnabled(true);
    mDurationLabel->setEnabled(true);
    mProgressSlider->setEnabled(true);
    mProgressSlider->setRange(0, duration);
    mDurationLabel->setText(QString("%1:%2:%3")
                                .arg(duration / 3600, 2, 10, QChar('0'))
                                .arg((duration % 3600) / 60, 2, 10, QChar('0'))
                                .arg((duration % 3600) % 60, 2, 10, QChar('0')));
    mParent->resetCursorHidden();
}

void ControllerWidget::setupUI(MainWindow* parent)
{
    mProgressSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mProgressSlider, SIGNAL(valueChanged(int)), this, SLOT(updateElapsedLabel(int)));
    connect(mProgressSlider, SIGNAL(mouseReleased(int)), parent, SLOT(seek(int)));

    mElapsedLabel = new QLabel("00:00:00");
    mDurationLabel = new QLabel("00::00::00");

    mChannelButton = new CustomButton(this, "CH: 1", 40, 24, "#fdfeff", "#3a97dc", "Channel Selection");
    connect(mChannelButton, SIGNAL(buttonPressed()), this, SLOT(popupChannelMenu()));

    mSkipBButton = new CustomButton(this, "../resources/Images/skipback.png", "../resources/Images/skipback_hov.png", "../resources/Images/skipback.png", "Skip Back");
    connect(mSkipBButton, SIGNAL(buttonPressed()), parent, SLOT(skipBack()));

    mSeekBButton = new CustomButton(this, "../resources/Images/seekback.png", "../resources/Images/seekback_hov.png", "../resources/Images/seekback.png", "Seek Back");
    connect(mSeekBButton, SIGNAL(buttonPressed()), parent, SLOT(seekBack()));

    mPlayPauseButton = new CustomButton(this, "../resources/Images/play.png", "../resources/Images/play_hov.png", "../resources/Images/play.png", "Play");
    connect(mPlayPauseButton, SIGNAL(buttonPressed()), parent, SLOT(playPause()));

    mStopButton = new CustomButton(this, "../resources/Images/stop.png", "../resources/Images/stop_hov.png", "../resources/Images/stop.png", "Stop");
    connect(mStopButton, SIGNAL(buttonPressed()), parent, SLOT(stop()));

    mSeekFButton = new CustomButton(this, "../resources/Images/seekforward.png", "../resources/Images/seekforward_hov.png", "../resources/Images/seekforward.png", "Seek Forward");
    connect(mSeekFButton, SIGNAL(buttonPressed()), parent, SLOT(seekForward()));

    mSkipFButton = new CustomButton(this, "../resources/Images/skipforward.png", "../resources/Images/skipforward_hov.png", "../resources/Images/skipforward.png", "Skip Forward");
    connect(mSkipFButton, SIGNAL(buttonPressed()), parent, SLOT(skipForward()));

    mVolumeButton = new CustomButton(this, "../resources/Images/volume.png", "../resources/Images/volume_hov.png", "../resources/Images/volume.png", "Mute");
    connect(mVolumeButton, SIGNAL(buttonPressed()), this, SLOT(volumePressed()));

    mVolumeSlider = new CustomSlider(Qt::Horizontal, this);
    connect(mVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateVolume(int)));
    mVolumeSlider->setRange(0, mPrevVolume);
    mVolumeSlider->setMaximumWidth(100);
    mVolumeSlider->setMinimumWidth(100);
    mVolumeSlider->updateSlider(mPrevVolume);

    mChannelMenu = new QMenu(mChannelButton);
    mChannelActionGroup = new QActionGroup(this);
    mChannelActionGroup->setExclusive(true);
    connect(mChannelMenu, SIGNAL(triggered(QAction*)), this, SLOT(channelMenuPressed(QAction*)));

    QHBoxLayout* hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(mElapsedLabel);
    hLayout1->addWidget(mProgressSlider);
    hLayout1->addWidget(mDurationLabel);

    QHBoxLayout* hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(mChannelButton);

    hLayout2->addItem(new QSpacerItem(0, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    hLayout2->addWidget(mSkipBButton);
    hLayout2->addWidget(mSeekBButton);
    hLayout2->addWidget(mPlayPauseButton);
    hLayout2->addWidget(mStopButton);
    hLayout2->addWidget(mSeekFButton);
    hLayout2->addWidget(mSkipFButton);
    hLayout2->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout2->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed, QSizePolicy::Minimum));
    hLayout2->addWidget(mVolumeButton);
    hLayout2->addWidget(mVolumeSlider);

    hLayout2->setSpacing(2);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout1);
    vLayout->addLayout(hLayout2);
    vLayout->setMargin(4);

    disableProgress();

    QString style = parent->loadStyleSheet("../style/ControllerStyle.css");
    setStyleSheet(style);
    setMaximumHeight(sizeHint().height());
}

void ControllerWidget::updateElapsedLabel(int elapsed)
{
    mElapsedLabel->setText(QString("%1:%2:%3")
                               .arg(elapsed / 3600, 2, 10, QChar('0'))
                               .arg((elapsed % 3600) / 60, 2, 10, QChar('0'))
                               .arg((elapsed % 3600) % 60, 2, 10, QChar('0')));
}

void ControllerWidget::updateVolume(int volume)
{
    if (volume == 0 && !mVolumeMuted)
        mute();
    else if (volume > 0 && mVolumeMuted)
        unmute();
    mParent->changeVolume(volume);
}

void ControllerWidget::volumePressed()
{
    mVolumeMuted = !mVolumeMuted;
    if (mVolumeMuted)
        mute();
    else
        unmute();
}

void ControllerWidget::mute()
{
    mPrevVolume = mVolumeSlider->value();
    if (mPrevVolume == 0)
        mPrevVolume = 100;
    mVolumeButton->setImage("../resources/Images/volumemute.png", "../resources/Images/volumemute_hov.png", "../resources/Images/volumemute.png", "Unmute");
    mVolumeSlider->updateSlider(0);
    mVolumeMuted = true;
}

void ControllerWidget::unmute()
{
    mVolumeButton->setImage("../resources/Images/volume.png", "../resources/Images/volume_hov.png", "../resources/Images/volume.png", "Mute");
    mVolumeSlider->updateSlider(mPrevVolume);
    mVolumeMuted = false;
}

void ControllerWidget::popupChannelMenu()
{
    mChannelMenu->popup(mChannelButton->mapToGlobal(QPoint(0, mChannelButton->height())));
}

void ControllerWidget::channelMenuPressed(QAction* action)
{
    QList<QAction*> actions = mChannelMenu->actions();
    for (int i = 0; i < actions.size(); i++) {
        if (actions[i] == action) {
            mChannelButton->setLabel("CH: " + QString::number(i + 1));
            mParent->changeChannel(i);
            break;
        }
    }
}

void ControllerWidget::updateChannelIndex(int index)
{
    QList<QAction*> actions = mChannelActionGroup->actions();
    for (int i = 0; i < actions.count(); i++) {
        if (i == index) {
            actions[i]->setChecked(true);
            mChannelButton->setLabel("CH: " + QString::number(i + 1));
            break;
        }
    }
}

void ControllerWidget::registerChannels(QList<ChannelInfo*> channels, int currentIndex)
{
    mChannelMenu->clear();
    QList<QAction*> lst = mChannelActionGroup->actions();
    for (int i = 0; i < lst.count(); i++) {
        mChannelActionGroup->removeAction(lst[i]);
        delete lst[i];
    }

    for (int i = 0; i < channels.count(); i++) {
        QAction* action = new QAction(channels[i]->getChannelName(), mChannelMenu);
        action->setCheckable(true);
        mChannelMenu->addAction(action);
        mChannelActionGroup->addAction(action);
        if (i == currentIndex) {
            action->setChecked(true);
            mChannelButton->setLabel("CH: " + QString::number(i + 1));
        }
    }
}

void ControllerWidget::volumeUp()
{
    int val = mVolumeSlider->value() + 5;
    if (val > 100)
        val = 100;
    updateVolume(val);
}

void ControllerWidget::volumeDown()
{
    int val = mVolumeSlider->value() - 5;
    if (val < 0)
        val = 0;
    updateVolume(val);
}
