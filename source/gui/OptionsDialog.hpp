#ifndef OPTIONSDIALOG_HPP
#define OPTIONSDIALOG_HPP

#include <QDialog>
#include <QTableWidgetItem>
#include <Windows.h>
class QTabWidget;
class MainWindow;
class RemoteControl;
class AlbumController;
class VideoRenderer;
class AudioRenderer;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QTableWidget;
class TcpCommand;
class TunerChannels;
#include "WinTuner.hpp"

class OptionsDialog : public QDialog {
    Q_OBJECT
public:
    OptionsDialog(MainWindow* parent, int remoteSize);
    ~OptionsDialog();

    void initGeneralTab();
    void loadGeneralSettings(MainWindow* mainWindow);
    void saveGeneralSettings(MainWindow* mainWindow);

    void initAlbumviewTab();
    void loadAlbumviewSettings(AlbumController* albumController);
    void saveAlbumviewSettings(AlbumController* albumController);

    void initVideoTab();
    void loadVideoSettings(VideoRenderer* renderer);
    void saveVideoSettings(VideoRenderer* renderer);

    void initAudioTab();
    void loadAudioSettings(AudioRenderer* audioRenderer);
    void saveAudioSettings(AudioRenderer* audioRenderer);

    void initTunerTab();
    void loadTunerSettings(TunerChannels* tunerChannels);
    void saveTunerSettings(TunerChannels* tunerChannels);

    void initNetworkTab();
    void loadNetworkSettings(TcpCommand* tcpCommand);
    void saveNetworkSettings(TcpCommand* tcpCommand);

    void initRemoteTab(int remoteSize);
    void loadRemoteSettings(RemoteControl* remote);
    void saveRemoteSettings(RemoteControl* remote);
    void remoteUpdateValue(RemoteControl* remote, BYTE* data, int sz);

    QTableWidget* getTableWidget();

signals:
    void displayErrorMessage(int errorLvl, const QString& msg);

public slots:
    void okPressed();
    void cancelPressed();

    // Tuner Settings Slots

    void moveUp();
    void moveDown();
    void removeChannels();
    void clearChannelList();

    void showTunerScannerWindow();
    void channelsReceived(QList<ChannelInfo*> channels);
    void ChannelItemChanged(QTableWidgetItem* item);
    void removeMenuPopup();
    void rawInputDeviceChanged(int index);

protected:
private slots:
    void showOverlayFontDialog();
    void showSubtitleFontDialog();
    void showEntityFontDialog();
    void showHeaderFontDialog();
    void showFooterFontDialog();

private:
    MainWindow* mParent;
    QTabWidget* tabWidget;
    QWidget* tunerTab;
    //OptionsAddChannel *addChannelDialog;

    // General Settings
    QWidget* generalTab;
    QLineEdit* fileFilter;

    // Albumview Settings
    QWidget* albumTab;
    QCheckBox* usePlaylistCheckBox;
    QCheckBox* autostartAlbumviewCheckBox;
    QLineEdit* maxiLineEdit;
    QLineEdit* maxjLineEdit;
    QLineEdit* mXSpacingLineEdit;
    QLineEdit* mYSpacingLineEdit;
    QLineEdit* mUMarginLineEdit;
    QLineEdit* mDMarginLineEdit;
    QLineEdit* mLMarginLineEdit;
    QLineEdit* mRMarginLineEdit;
    QLineEdit* scrollSpeedLineEdit;
    QLineEdit* thumnailTimeLineEdit;

    QLineEdit* fontEntity;
    QComboBox* comboColorEntity;
    QComboBox* comboSizeEntity;

    QLineEdit* fontHeader;
    QComboBox* comboColorHeader;
    QComboBox* comboSizeHeader;

    QLineEdit* fontFooter;
    QComboBox* comboColorFooter;
    QComboBox* comboSizeFooter;

    // Video Settings
    QWidget* videoTab;
    QLineEdit* fontSubtitle;
    QComboBox* comboColorSubtitle;
    QComboBox* comboSizeSubtitle;
    QLineEdit *xPosSubtitle, *yPosSubtitle;

    QLineEdit* fontOverlay;
    QComboBox* comboColorOverlay;
    QComboBox* comboSizeOverlay;
    QLineEdit *xPosOverlay, *yPosOverlay;
    QLineEdit* fadetimeOverlay;

    // Audio Settings
    QWidget* audioTab;
    QComboBox* audioDevice;
    QLineEdit* audioBuffers;
    QCheckBox* audioCompensate;

    // Remote Settings
    QWidget* remoteTab;
    QLabel** remoteLabel;
    QLineEdit** remoteLineEdit;
    QComboBox* inputDeviceCombo;
    QLineEdit* debugLineEdit;
    QLineEdit* maskLineEdit;
    //QCheckBox *useDevice;
    QLineEdit* usageLineEdit;
    QLineEdit* usagePageLineEdit;

    // Network Settings
    QWidget* networkTab;
    QLineEdit* listenPortLineEdit;
    QLineEdit* streamPortLineEdit;
    QLineEdit* serverPassLineEdit;

    // Tuner Settings
    QTableWidget* channelTableWidget;
    QList<ChannelInfo*> channels;
    QPushButton* removeChannel;
    QMenu* removeMenu;
};

#endif
