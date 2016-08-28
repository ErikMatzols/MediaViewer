#include "OptionsDialog.hpp"
#include "AlbumController.hpp"
#include "AlbumModel.hpp"
#include "AudioRenderer.hpp"
#include "MainWindow.hpp"
#include "RemoteControl.hpp"
#include "TcpCommand.hpp"
#include "VideoRenderer.hpp"
#include "WinTunerScanner.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

void enumerateRawInput(QComboBox* /*comboBox*/)
{
    UINT nDevices;
    PRAWINPUTDEVICELIST pRawInputDeviceList;
    if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
        return;
    if ((pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * nDevices)) == NULL)
        return;

    int nNoOfDevices = 0;
    if ((nNoOfDevices = GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST))) == ((UINT)-1))
        return;

    RID_DEVICE_INFO rdi;
    rdi.cbSize = sizeof(RID_DEVICE_INFO);

    for (int i = 0; i < nNoOfDevices; i++) {
        UINT size = 256;
        TCHAR tBuffer[256] = { 0 };

        if (GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, tBuffer, &size) < 0) {
            // Error in reading device name
        }

        // cout << "Device Name: " << tBuffer << endl;

        UINT cbSize = rdi.cbSize;
        if (GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) < 0) {
            // Error in reading information
        }
        if (rdi.dwType == RIM_TYPEHID) {
            //comboBox->addItem(QString::fromUtf16(tBuffer),QVariant(QSize(rdi.hid.usUsage,rdi.hid.usUsagePage)));
            /*
            _tprintf(L"Device Name: %s\n", tBuffer);
			cout << "Vendor Id:" << rdi.hid.dwVendorId << endl;
			cout << "Product Id:" << rdi.hid.dwProductId << endl;
			cout << "Version No:" << rdi.hid.dwVersionNumber << endl;
			cout << "Usage for the device: " << rdi.hid.usUsage << endl;
			cout << "Usage Page for the device: " << rdi.hid.usUsagePage << endl;
			cout << "***********************" << endl;
            */
        }
    }

    free(pRawInputDeviceList);
}

OptionsDialog::OptionsDialog(MainWindow* parent, int remoteSize)
    : QDialog(parent)
{
    mParent = parent;
    tabWidget = new QTabWidget(this);
    generalTab = new QWidget;
    albumTab = new QWidget;
    videoTab = new QWidget;
    audioTab = new QWidget;
    tunerTab = new QWidget;
    networkTab = new QWidget;
    remoteTab = new QWidget;

    tabWidget->addTab(generalTab, "General");
    tabWidget->addTab(albumTab, "Albumview");
    tabWidget->addTab(videoTab, "Video");
    tabWidget->addTab(audioTab, "Audio");
    tabWidget->addTab(tunerTab, "Tuner");
    tabWidget->addTab(networkTab, "Network");
    tabWidget->addTab(remoteTab, "Remote");

    QVBoxLayout* vertLayout = new QVBoxLayout(this);
    vertLayout->addWidget(tabWidget);

    QHBoxLayout* horLayout = new QHBoxLayout;
    QSpacerItem* horSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horLayout->addItem(horSpacer);

    QPushButton* okButton = new QPushButton("Ok", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    horLayout->addWidget(okButton);
    horLayout->addWidget(cancelButton);

    vertLayout->addLayout(horLayout);

    remoteLabel = NULL;
    remoteLineEdit = NULL;

    initGeneralTab();
    initAlbumviewTab();
    initVideoTab();
    initAudioTab();
    initTunerTab();
    initNetworkTab();
    initRemoteTab(remoteSize);

    setWindowTitle("Options");
    resize(800, 300);
}

OptionsDialog::~OptionsDialog()
{
    delete[] remoteLabel;
    delete[] remoteLineEdit;
}

void OptionsDialog::okPressed()
{
    accept();
}

void OptionsDialog::cancelPressed()
{
    reject();
}

void OptionsDialog::initGeneralTab()
{
    QVBoxLayout* vLayout = new QVBoxLayout(generalTab);
    QHBoxLayout* hLayout = new QHBoxLayout;

    QGridLayout* gridLayout = new QGridLayout();

    QLabel* label_1 = new QLabel("File Filter");
    fileFilter = new QLineEdit(generalTab);
    gridLayout->addWidget(label_1, 1, 0, 1, 1);
    gridLayout->addWidget(fileFilter, 1, 3, 1, 1);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addLayout(gridLayout);
    hLayout->addItem(horizontalSpacer);
    vLayout->addLayout(hLayout);
}

void OptionsDialog::loadGeneralSettings(MainWindow* mainWindow)
{
    fileFilter->setText(mainWindow->fileFilter());
}

void OptionsDialog::saveGeneralSettings(MainWindow* mainWindow)
{
    mainWindow->setFileFilter(fileFilter->text());
}

void OptionsDialog::initAlbumviewTab()
{
    QScrollArea* scrollArea = new QScrollArea(albumTab);

    QGridLayout* gridLayout = new QGridLayout(scrollArea);

    gridLayout->addWidget(new QLabel("Album x"), 0, 0, 1, 1);
    maxiLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(maxiLineEdit, 0, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Album y"), 1, 0, 1, 1);
    maxjLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(maxjLineEdit, 1, 1, 1, 1);

    QVBoxLayout* verticalLayout = new QVBoxLayout(albumTab);
    verticalLayout->addWidget(scrollArea);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    gridLayout->addWidget(new QLabel("Spacing x"), 2, 0, 1, 1);
    mXSpacingLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mXSpacingLineEdit, 2, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Spacing y"), 3, 0, 1, 1);
    mYSpacingLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mYSpacingLineEdit, 3, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Margin u"), 4, 0, 1, 1);
    mUMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mUMarginLineEdit, 4, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Margin b"), 5, 0, 1, 1);
    mDMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mDMarginLineEdit, 5, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Margin l"), 6, 0, 1, 1);
    mLMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mLMarginLineEdit, 6, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Margin r"), 7, 0, 1, 1);
    mRMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mRMarginLineEdit, 7, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Text scroll speed"), 8, 0, 1, 1);
    scrollSpeedLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(scrollSpeedLineEdit, 8, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Thumbnail extract time"), 9, 0, 1, 1);
    thumnailTimeLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(thumnailTimeLineEdit, 9, 1, 1, 1);

    gridLayout->addWidget(new QLabel("Entity font", albumTab), 10, 0, 1, 1);
    fontEntity = new QLineEdit(albumTab);
    gridLayout->addWidget(fontEntity, 10, 1, 1, 1);
    QPushButton* button_1 = new QPushButton("Browse...");
    connect(button_1, SIGNAL(clicked()), this, SLOT(showEntityFontDialog()));
    gridLayout->addWidget(button_1, 10, 2, 1, 1);

    comboColorEntity = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Entity font color", albumTab), 11, 0, 1, 1);
    gridLayout->addWidget(comboColorEntity, 11, 1, 1, 1);

    comboSizeEntity = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Entity font size", albumTab), 12, 0, 1, 1);
    gridLayout->addWidget(comboSizeEntity, 12, 1, 1, 1);

    /*
	QVBoxLayout *verticalLayout = new QVBoxLayout(albumTab);
	QHBoxLayout *horizontalLayout = new QHBoxLayout();
	QGridLayout *gridLayout = new QGridLayout();

	QLabel *maxiLabel = new QLabel("xAlbums",albumTab);
	gridLayout->addWidget(maxiLabel, 0, 0, 1, 1);
	maxiLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(maxiLineEdit, 0, 3, 1, 1);

	QLabel *maxJLabel = new QLabel("yAlbums",albumTab);
	gridLayout->addWidget(maxJLabel, 1, 0, 1, 1);
	maxjLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(maxjLineEdit, 1, 3, 1, 1);

	QLabel *xSpacingLabel = new QLabel("xSpacing",albumTab);
	gridLayout->addWidget(xSpacingLabel, 2, 0, 1, 2);
	mXSpacingLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(mXSpacingLineEdit, 2, 3, 1, 1);

	QLabel *ySpacingLabel = new QLabel("ySpacing",albumTab);
	gridLayout->addWidget(ySpacingLabel, 4, 0, 1, 2);
	mYSpacingLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(mYSpacingLineEdit, 4, 3, 1, 1);

	QLabel *uMarginLabel = new QLabel("uMargin",albumTab);
	gridLayout->addWidget(uMarginLabel, 5, 0, 1, 2);
	mUMarginLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(mUMarginLineEdit, 5, 3, 1, 1);

    QLabel *bMarginLabel = new QLabel("bMargin",albumTab);
    gridLayout->addWidget(bMarginLabel, 6, 0, 1, 2);
    mDMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mDMarginLineEdit, 6, 3, 1, 1);

    QLabel *lMarginLabel = new QLabel("lMargin",albumTab);
    gridLayout->addWidget(lMarginLabel, 7, 0, 1, 2);
    mLMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mLMarginLineEdit, 7, 3, 1, 1);

    QLabel *rMarginLabel = new QLabel("rMargin",albumTab);
    gridLayout->addWidget(rMarginLabel, 8, 0, 1, 2);
    mRMarginLineEdit = new QLineEdit(albumTab);
    gridLayout->addWidget(mRMarginLineEdit, 8, 3, 1, 1);
    
	QLabel *textScrollSpeedLabel = new QLabel("Text Scroll Speed",albumTab);
	gridLayout->addWidget(textScrollSpeedLabel, 9, 0, 1, 3);
	scrollSpeedLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(scrollSpeedLineEdit, 9, 3, 1, 1);

	QLabel *thumbnailTimeLabel = new QLabel("Extract Time",albumTab);
	gridLayout->addWidget(thumbnailTimeLabel,10,0,1,3);
	thumnailTimeLineEdit = new QLineEdit(albumTab);
	gridLayout->addWidget(thumnailTimeLineEdit,10,3,1,1);

	QLabel *usePlaylistLabel = new QLabel("Use Playlist",albumTab);
	gridLayout->addWidget(usePlaylistLabel, 11, 0, 1, 1);
	usePlaylistCheckBox = new QCheckBox(albumTab);
	gridLayout->addWidget(usePlaylistCheckBox, 11, 3, 1, 1);

    QLabel *startAlbumLabel = new QLabel("Autostart Albumview",albumTab);
    gridLayout->addWidget(startAlbumLabel,12,0,1,1);
    autostartAlbumviewCheckBox = new QCheckBox(albumTab);
    gridLayout->addWidget(autostartAlbumviewCheckBox,12,3,1,1);

    fontEntity = new QLineEdit(albumTab);
    gridLayout->addWidget(new QLabel("Entity Font", albumTab), 0, 4, 1, 1);
    gridLayout->addWidget(fontEntity, 0, 5, 1, 1);
    QPushButton *button_1 = new QPushButton("Browse...");
    connect(button_1, SIGNAL(clicked()), this, SLOT(showEntityFontDialog()));
    gridLayout->addWidget(button_1, 0, 6, 1, 1);

    comboColorEntity = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Entity Font Color", albumTab), 1, 4, 1, 1);
    gridLayout->addWidget(comboColorEntity, 1, 5, 1, 1);

    comboSizeEntity = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Entity Font Size", albumTab), 2, 4, 1, 1);
    gridLayout->addWidget(comboSizeEntity, 2, 5, 1, 1);


    fontHeader = new QLineEdit(albumTab);
    gridLayout->addWidget(new QLabel("Header Font", albumTab), 3, 4, 1, 1);
    gridLayout->addWidget(fontHeader, 3, 5, 1, 1);
    QPushButton *button_2 = new QPushButton("Browse...");
    connect(button_2, SIGNAL(clicked()), this, SLOT(showHeaderFontDialog()));
    gridLayout->addWidget(button_2, 3, 6, 1, 1);

    comboColorHeader = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Header Font Color", albumTab), 4, 4, 1, 1);
    gridLayout->addWidget(comboColorHeader, 4, 5, 1, 1);

    comboSizeHeader = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Header Font Size", albumTab), 5, 4, 1, 1);
    gridLayout->addWidget(comboSizeHeader, 5, 5, 1, 1);



    fontFooter = new QLineEdit(albumTab);
    gridLayout->addWidget(new QLabel("Header Font", albumTab), 6, 4, 1, 1);
    gridLayout->addWidget(fontFooter, 6, 5, 1, 1);
    QPushButton *button_3 = new QPushButton("Browse...");
    connect(button_3, SIGNAL(clicked()), this, SLOT(showFooterFontDialog()));
    gridLayout->addWidget(button_3, 6, 6, 1, 1);

    comboColorFooter = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Header Font Color", albumTab), 7, 4, 1, 1);
    gridLayout->addWidget(comboColorFooter, 7, 5, 1, 1);

    comboSizeFooter = new QComboBox(albumTab);
    gridLayout->addWidget(new QLabel("Header Font Size", albumTab), 8, 4, 1, 1);
    gridLayout->addWidget(comboSizeFooter, 8, 5, 1, 1);

	horizontalLayout->addLayout(gridLayout);
	QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);
	verticalLayout->addLayout(horizontalLayout);
	QSpacerItem *verticalSpacer = new QSpacerItem(20, 217, QSizePolicy::Minimum, QSizePolicy::Expanding);
	verticalLayout->addItem(verticalSpacer);

    */
}

void OptionsDialog::loadAlbumviewSettings(AlbumController* /*albumController*/)
{
    /*
    AlbumAlignment alignment = albumController->getLocalModel()->albumAlignment();
    maxiLineEdit->setText(QString::number(alignment.maxI));
    maxjLineEdit->setText(QString::number(alignment.maxJ));
    mXSpacingLineEdit->setText(QString::number(alignment.xSpacing));
    mYSpacingLineEdit->setText(QString::number(alignment.ySpacing));
    mUMarginLineEdit->setText(QString::number(alignment.uMargin));
    mDMarginLineEdit->setText(QString::number(alignment.dMargin));
    mLMarginLineEdit->setText(QString::number(alignment.lMargin));
    mRMarginLineEdit->setText(QString::number(alignment.rMargin));
    scrollSpeedLineEdit->setText(QString::number(albumController->scrollSpeed()));
    thumnailTimeLineEdit->setText(QString::number(albumController->thumbnailExtractTime()));
    usePlaylistCheckBox->setChecked(albumController->playlistEnabled());
    autostartAlbumviewCheckBox->setChecked(albumController->autoStartEnabled());
    */
}

void OptionsDialog::saveAlbumviewSettings(AlbumController* albumController)
{
    AlbumModel* model = albumController->getLocalModel();
    model->setDimension(maxiLineEdit->text().toInt(), maxjLineEdit->text().toInt());
    model->setSpacing(mXSpacingLineEdit->text().toInt(), mYSpacingLineEdit->text().toInt());
    model->setMargin(mUMarginLineEdit->text().toInt(),
        mDMarginLineEdit->text().toInt(),
        mLMarginLineEdit->text().toInt(),
        mRMarginLineEdit->text().toInt());
    albumController->setScrollSpeed(scrollSpeedLineEdit->text().toFloat());
    albumController->setThumbnailExtractTime(thumnailTimeLineEdit->text().toInt());
    albumController->setPlaylistEnabled(usePlaylistCheckBox->isChecked());
    albumController->setAutoStartEnabled(autostartAlbumviewCheckBox->isChecked());
}

void OptionsDialog::initVideoTab()
{
    QVBoxLayout* vLayout = new QVBoxLayout(videoTab);
    QHBoxLayout* hLayout = new QHBoxLayout;

    QGridLayout* gridLayout = new QGridLayout();

    QLabel* label_1 = new QLabel("Overlay Font");
    fontOverlay = new QLineEdit(videoTab);
    QPushButton* button_1 = new QPushButton("Browse...");
    connect(button_1, SIGNAL(clicked()), this, SLOT(showOverlayFontDialog()));
    gridLayout->addWidget(label_1, 0, 0, 1, 1);
    gridLayout->addWidget(fontOverlay, 0, 3, 1, 1);
    gridLayout->addWidget(button_1, 0, 4, 1, 1);

    QLabel* label_2 = new QLabel("Overlay Color");
    comboColorOverlay = new QComboBox(videoTab);
    comboColorOverlay->insertItems(0, QStringList() << "White"
                                                    << "Black"
                                                    << "Red"
                                                    << "Green"
                                                    << "Yellow"
                                                    << "Blue"
                                                    << "Brown"
                                                    << "Orange"
                                                    << "Pink"
                                                    << "Purple"
                                                    << "Gray");
    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    gridLayout->addWidget(comboColorOverlay, 1, 3, 1, 1);

    QLabel* label_3 = new QLabel("Overlay Size");
    comboSizeOverlay = new QComboBox(videoTab);
    comboSizeOverlay->insertItems(0, QStringList() << "10"
                                                   << "12"
                                                   << "14"
                                                   << "16"
                                                   << "18"
                                                   << "20"
                                                   << "22"
                                                   << "24"
                                                   << "26"
                                                   << "28"
                                                   << "30"
                                                   << "32"
                                                   << "34"
                                                   << "36"
                                                   << "38"
                                                   << "40"
                                                   << "42"
                                                   << "44"
                                                   << "46"
                                                   << "48"
                                                   << "50");
    gridLayout->addWidget(label_3, 2, 0, 1, 1);
    gridLayout->addWidget(comboSizeOverlay, 2, 3, 1, 1);

    QLabel* label_4 = new QLabel("Overlay Horisontal Pos");
    xPosOverlay = new QLineEdit(videoTab);
    gridLayout->addWidget(label_4, 3, 0, 1, 1);
    gridLayout->addWidget(xPosOverlay, 3, 3, 1, 1);

    QLabel* label_5 = new QLabel("Overlay Vertical Pos");
    yPosOverlay = new QLineEdit(videoTab);
    gridLayout->addWidget(label_5, 4, 0, 1, 1);
    gridLayout->addWidget(yPosOverlay, 4, 3, 1, 1);

    QLabel* label_99 = new QLabel("Overlay Fadeout Time");
    fadetimeOverlay = new QLineEdit(videoTab);
    gridLayout->addWidget(label_99, 5, 0, 1, 1);
    gridLayout->addWidget(fadetimeOverlay, 5, 3, 1, 1);

    QLabel* label_6 = new QLabel("Subtitle Font");
    fontSubtitle = new QLineEdit(videoTab);
    QPushButton* button_2 = new QPushButton("Browse...");
    connect(button_2, SIGNAL(clicked()), this, SLOT(showSubtitleFontDialog()));
    gridLayout->addWidget(label_6, 6, 0, 1, 1);
    gridLayout->addWidget(fontSubtitle, 6, 3, 1, 1);
    gridLayout->addWidget(button_2, 6, 4, 1, 1);

    QLabel* label_7 = new QLabel("Subtitle Color");
    comboColorSubtitle = new QComboBox(videoTab);
    comboColorSubtitle->insertItems(0, QStringList() << "White"
                                                     << "Black"
                                                     << "Red"
                                                     << "Green"
                                                     << "Yellow"
                                                     << "Blue"
                                                     << "Brown"
                                                     << "Orange"
                                                     << "Pink"
                                                     << "Purple"
                                                     << "Gray");
    gridLayout->addWidget(label_7, 7, 0, 1, 1);
    gridLayout->addWidget(comboColorSubtitle, 7, 3, 1, 1);

    QLabel* label_8 = new QLabel("Subtitle Size");
    comboSizeSubtitle = new QComboBox(videoTab);
    comboSizeSubtitle->insertItems(0, QStringList() << "10"
                                                    << "12"
                                                    << "14"
                                                    << "16"
                                                    << "18"
                                                    << "20"
                                                    << "22"
                                                    << "24"
                                                    << "26"
                                                    << "28"
                                                    << "30"
                                                    << "32"
                                                    << "34"
                                                    << "36"
                                                    << "38"
                                                    << "40"
                                                    << "42"
                                                    << "44"
                                                    << "46"
                                                    << "48"
                                                    << "50");
    gridLayout->addWidget(label_8, 8, 0, 1, 1);
    gridLayout->addWidget(comboSizeSubtitle, 8, 3, 1, 1);

    QLabel* label_9 = new QLabel("Subtitle Horisontal Pos");
    xPosSubtitle = new QLineEdit(videoTab);
    gridLayout->addWidget(label_9, 9, 0, 1, 1);
    gridLayout->addWidget(xPosSubtitle, 9, 3, 1, 1);

    QLabel* label_10 = new QLabel("Subtitle Vertical Pos");
    yPosSubtitle = new QLineEdit(videoTab);
    gridLayout->addWidget(label_10, 10, 0, 1, 1);
    gridLayout->addWidget(yPosSubtitle, 10, 3, 1, 1);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addLayout(gridLayout);
    hLayout->addItem(horizontalSpacer);
    vLayout->addLayout(hLayout);
}

void OptionsDialog::loadVideoSettings(VideoRenderer* renderer)
{
    QString txt, txtColor;
    int size;
    QPoint pos;
    float fadeTime;

    renderer->getOverlayFont(txt, txtColor, size, pos, fadeTime);

    fontOverlay->setText(txt);
    comboColorOverlay->setCurrentIndex(comboColorOverlay->findText(txtColor));
    comboSizeOverlay->setCurrentIndex(comboSizeOverlay->findText(QString::number(size)));
    xPosOverlay->setText(QString::number(pos.x()));
    yPosOverlay->setText(QString::number(pos.y()));
    fadetimeOverlay->setText(QString::number(fadeTime, 'g'));

    renderer->getSubtitleFont(txt, txtColor, size, pos);
    fontSubtitle->setText(txt);
    comboColorSubtitle->setCurrentIndex(comboColorSubtitle->findText(txtColor));
    comboSizeSubtitle->setCurrentIndex(comboSizeSubtitle->findText(QString::number(size)));
    xPosSubtitle->setText(QString::number(pos.x()));
    yPosSubtitle->setText(QString::number(pos.y()));
}

void OptionsDialog::saveVideoSettings(VideoRenderer* renderer)
{
    bool ok = true;
    renderer->setOverlayFont(fontOverlay->text(), comboColorOverlay->currentText(), comboSizeOverlay->currentText().toInt(&ok),
        QPoint(xPosOverlay->text().toInt(&ok), yPosOverlay->text().toInt(&ok)),
        fadetimeOverlay->text().toFloat(&ok));

    renderer->setSubtitleFont(fontSubtitle->text(), comboColorSubtitle->currentText(), comboSizeSubtitle->currentText().toInt(&ok),
        QPoint(xPosSubtitle->text().toInt(&ok), yPosSubtitle->text().toInt(&ok)));
}

void OptionsDialog::initAudioTab()
{
    QVBoxLayout* vLayout = new QVBoxLayout(audioTab);
    QHBoxLayout* hLayout = new QHBoxLayout;

    QGridLayout* gridLayout = new QGridLayout();

    QLabel* label_1 = new QLabel("Audio Device");
    audioDevice = new QComboBox(audioTab);
    gridLayout->addWidget(label_1, 0, 0, 1, 1);
    gridLayout->addWidget(audioDevice, 0, 3, 1, 1);

    QLabel* label_2 = new QLabel("Audio Buffers");
    audioBuffers = new QLineEdit(audioTab);

    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    gridLayout->addWidget(audioBuffers, 1, 3, 1, 1);

    QLabel* label_3 = new QLabel("Compensate for Buffer Latency");
    audioCompensate = new QCheckBox(audioTab);

    gridLayout->addWidget(label_3, 2, 0, 1, 1);
    gridLayout->addWidget(audioCompensate, 2, 3, 1, 1);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addLayout(gridLayout);
    hLayout->addItem(horizontalSpacer);
    vLayout->addLayout(hLayout);
}

void OptionsDialog::loadAudioSettings(AudioRenderer* audioRenderer)
{
    audioDevice->insertItem(0, QString::fromUtf8(audioRenderer->getDefaultDeviceName()));
    audioBuffers->setText(QString::number(audioRenderer->getBufferCount()));
    audioCompensate->setChecked(audioRenderer->compensateBufferLatency());
}

void OptionsDialog::saveAudioSettings(AudioRenderer* audioRenderer)
{
    audioRenderer->setBufferCount(audioBuffers->text().toInt());
    audioRenderer->setCompensateLatency(audioCompensate->isChecked());
}

void OptionsDialog::initRemoteTab(int remoteSize)
{
    QWidget* leftWidget = new QWidget(remoteTab);
    leftWidget->setMinimumWidth(200);
    leftWidget->setMaximumWidth(200);
    QScrollArea* scrollArea = new QScrollArea(remoteTab);
    scrollArea->setWidgetResizable(true);
    QWidget* scrollAreaWidget = new QWidget(remoteTab);

    QHBoxLayout* hLayoutWidget = new QHBoxLayout(remoteTab);
    QHBoxLayout* hLayoutLeftWidget = new QHBoxLayout(leftWidget);

    QWidget* infoWidget = new QWidget(leftWidget);
    QGridLayout* gLayoutInfoWidget = new QGridLayout;
    gLayoutInfoWidget->setContentsMargins(0, 0, 0, 0);
    hLayoutLeftWidget->addWidget(infoWidget);

    QLabel* usageLabel = new QLabel("Usage", infoWidget);
    QLabel* usagePageLabel = new QLabel("UsagePage", infoWidget);
    usageLineEdit = new QLineEdit(infoWidget);
    usageLineEdit->setReadOnly(true);
    usagePageLineEdit = new QLineEdit(infoWidget);
    usagePageLineEdit->setReadOnly(true);

    QLabel* debugLabel = new QLabel("Debug", this);
    debugLineEdit = new QLineEdit(this);
    debugLineEdit->setReadOnly(true);
    QLabel* maskLabel = new QLabel("Mask", this);
    maskLineEdit = new QLineEdit(this);

    inputDeviceCombo = new QComboBox(this);
    connect(inputDeviceCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(rawInputDeviceChanged(int)));
    inputDeviceCombo->addItem("No Device", QVariant());
    enumerateRawInput(inputDeviceCombo);

    gLayoutInfoWidget->addWidget(debugLabel, 0, 0);
    gLayoutInfoWidget->addWidget(debugLineEdit, 0, 1);
    gLayoutInfoWidget->addWidget(maskLabel, 1, 0);
    gLayoutInfoWidget->addWidget(maskLineEdit, 1, 1);
    gLayoutInfoWidget->addWidget(usageLabel, 2, 0);
    gLayoutInfoWidget->addWidget(usageLineEdit, 2, 1);
    gLayoutInfoWidget->addWidget(usagePageLabel, 3, 0);
    gLayoutInfoWidget->addWidget(usagePageLineEdit, 3, 1);

    QVBoxLayout* layoutleft = new QVBoxLayout;
    layoutleft->addWidget(inputDeviceCombo);
    layoutleft->addLayout(gLayoutInfoWidget);
    layoutleft->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));

    hLayoutWidget->addLayout(layoutleft);
    hLayoutWidget->addWidget(scrollArea);

    QGridLayout* gridLayout = new QGridLayout(scrollAreaWidget);
    remoteLabel = new QLabel*[remoteSize];
    remoteLineEdit = new QLineEdit*[remoteSize];
    for (int i = 0; i < remoteSize; i++) {
        remoteLabel[i] = new QLabel();
        remoteLineEdit[i] = new QLineEdit(scrollAreaWidget);
        gridLayout->addWidget(remoteLabel[i], i, 0);
        gridLayout->addWidget(remoteLineEdit[i], i, 1);
    }

    scrollArea->setWidget(scrollAreaWidget);
}

void OptionsDialog::loadRemoteSettings(RemoteControl* remote)
{
    for (int i = 0; i < inputDeviceCombo->count(); i++) {
        QVariant v = inputDeviceCombo->itemData(i);
        if (v.canConvert(QVariant::Size)) {
            QSize sz = v.toSize();
            if (remote->getUsage() == sz.width() && remote->getUsagePage() == sz.height()) {
                inputDeviceCombo->setCurrentIndex(i);
                maskLineEdit->setText(remote->getInputMask());
            }
        }
    }

    QMap<QString, RemoteLookup>& map = remote->getMap();
    QMap<QString, RemoteLookup>::iterator i = map.begin();
    int nr = 0;
    while (i != map.end()) {
        remoteLabel[nr]->setText(i.key());
        remoteLineEdit[nr]->setText(QString::number(i->id));
        ++i;
        ++nr;
    }
}

void OptionsDialog::saveRemoteSettings(RemoteControl* remote)
{
    bool tmp;
    if (inputDeviceCombo->currentText().compare("No Device") != 0)
        remote->setUsed(true);
    remote->setInputMask(maskLineEdit->text());
    remote->registerDevice(mParent, usageLineEdit->text().toUShort(&tmp, 10),
        usagePageLineEdit->text().toUShort(&tmp, 10));
    QMap<QString, RemoteLookup>& map = remote->getMap();
    QMap<QString, RemoteLookup>::iterator i = map.begin();
    int nr = 0;
    while (i != map.end()) {
        RemoteLookup lookup = i.value();
        lookup.id = remoteLineEdit[nr]->text().toInt();
        map.insert(i.key(), lookup);
        ++i;
        ++nr;
    }
}

void OptionsDialog::remoteUpdateValue(RemoteControl* remote, BYTE* data, int sz)
{
    QString debugStr;
    int id = -1;
    for (int i = 0; i < sz; i++) {
        QString hex;
        if (data[i] < 16)
            hex += "0";
        debugStr += hex + QString::number(data[i], 16) + " ";
    }
    debugLineEdit->setText(debugStr);

    QStringList mask = maskLineEdit->text().split(" ");
    if (mask.count() == sz) {
        for (int i = 0; i < sz; i++)
            if (mask[i].compare("FF") == 0)
                id += data[i];
    }
    std::cout << "id: " << id << "\n";

    QMap<QString, RemoteLookup>& map = remote->getMap();
    QMap<QString, RemoteLookup>::iterator i = map.begin();
    int nr = 0;
    while (i != map.end()) {
        if (remoteLineEdit[nr]->hasFocus())
            remoteLineEdit[nr]->setText(QString::number(id));
        ++i;
        ++nr;
    }
}

void OptionsDialog::rawInputDeviceChanged(int index)
{
    QVariant v = inputDeviceCombo->itemData(index);
    if (v.canConvert(QVariant::Size)) {
        QSize size = v.toSize();
        usageLineEdit->setText(QString::number(size.width()));
        usagePageLineEdit->setText(QString::number(size.height()));
    } else {
        usageLineEdit->setText(QString::number(0));
        usagePageLineEdit->setText(QString::number(0));
    }
}

void OptionsDialog::initTunerTab()
{
    channelTableWidget = new QTableWidget(tunerTab);
    channelTableWidget->horizontalHeader()->setStretchLastSection(true);
    channelTableWidget->setAlternatingRowColors(true);
    channelTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    channelTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    channelTableWidget->verticalHeader()->setDefaultSectionSize(20);
    channelTableWidget->setColumnCount(2);

    channelTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Channel"));
    channelTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Device"));

    QPushButton* scannerButton = new QPushButton("Scanner");
    connect(scannerButton, SIGNAL(clicked()), this, SLOT(showTunerScannerWindow()));

    removeChannel = new QPushButton("Remove");
    connect(removeChannel, SIGNAL(clicked()), this, SLOT(removeMenuPopup()));

    QVBoxLayout* vLeftButtonsLayout = new QVBoxLayout;
    vLeftButtonsLayout->addWidget(scannerButton);
    vLeftButtonsLayout->addWidget(removeChannel);

    removeMenu = new QMenu(removeChannel);
    removeMenu->addAction("Remove Selected Channel(s)", this, SLOT(removeChannels()));
    removeMenu->addAction("Clear Channel List", this, SLOT(clearChannelList()));

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton* moveUpButton = new QPushButton("Move Up");
    QPushButton* moveDownButton = new QPushButton("Move Down");
    QVBoxLayout* vRightButtonsLayout = new QVBoxLayout;
    vRightButtonsLayout->addWidget(moveUpButton);
    vRightButtonsLayout->addWidget(moveDownButton);

    QHBoxLayout* hButtonsLayout = new QHBoxLayout;
    hButtonsLayout->addLayout(vLeftButtonsLayout);
    hButtonsLayout->addItem(horizontalSpacer);
    hButtonsLayout->addLayout(vRightButtonsLayout);

    QVBoxLayout* vLayout = new QVBoxLayout(tunerTab);
    vLayout->addWidget(channelTableWidget);
    vLayout->addLayout(hButtonsLayout);

    connect(channelTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(ChannelItemChanged(QTableWidgetItem*)));
    connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));
}

void OptionsDialog::loadTunerSettings(TunerChannels* tunerChannels)
{
    channels.clear(); // should not leak because its shared with tunerChannels

    foreach (ChannelInfo* channel, tunerChannels->getChannelList())
        channels.append(channel->copy());

    channelTableWidget->clearContents();
    channelTableWidget->setRowCount(channels.size());
    for (int i = 0; i < channels.size(); i++) {
        ChannelInfo* info = channels.at(i);
        channelTableWidget->setItem(i, 0, new QTableWidgetItem(info->getChannelName()));
        QTableWidgetItem* item = new QTableWidgetItem(info->getDeviceName());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        channelTableWidget->setItem(i, 1, item);
    }
}

void OptionsDialog::saveTunerSettings(TunerChannels* tunerChannels)
{
    tunerChannels->setChannelList(channels);
    channels.clear();
}

void OptionsDialog::showTunerScannerWindow()
{
    WinTunerScanner scanner(this);
    scanner.setWindowTitle("Scanner");
    connect(&scanner, SIGNAL(sendChannels(QList<ChannelInfo*>)), this, SLOT(channelsReceived(QList<ChannelInfo*>)));
    scanner.exec();
}

void OptionsDialog::channelsReceived(QList<ChannelInfo*> channels)
{
    this->channels.append(channels);
    foreach (ChannelInfo* channel, channels) {
        int row = channelTableWidget->rowCount();
        channelTableWidget->setRowCount(row + 1);
        channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
        QTableWidgetItem* item = new QTableWidgetItem(channel->getDeviceName());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        channelTableWidget->setItem(row, 1, item);
    }
}

QTableWidget* OptionsDialog::getTableWidget()
{
    return channelTableWidget;
}

void OptionsDialog::ChannelItemChanged(QTableWidgetItem* item)
{
    if (item->flags() & Qt::ItemIsEditable) {
        channels[(item->row())]->setChannelName(item->text());
    }
}

bool sortByTopRow(const QTableWidgetSelectionRange range1, const QTableWidgetSelectionRange range2)
{
    return range1.topRow() < range2.topRow();
}

void OptionsDialog::moveUp()
{
    int newRow = -1;
    QList<QTableWidgetSelectionRange> selections = channelTableWidget->selectedRanges();
    qSort(selections.begin(), selections.end(), sortByTopRow);
    foreach (QTableWidgetSelectionRange selection, selections) {
        for (int i = selection.topRow(); i <= selection.bottomRow(); i++) {
            newRow = i - 1;
            if (newRow >= 0)
                channels.move(i, newRow);
            else
                break;
        }
    }
    if (newRow >= 0) {
        channelTableWidget->clearContents();
        channelTableWidget->setRowCount(0);
        channelTableWidget->blockSignals(true);
        foreach (ChannelInfo* channel, channels) {
            int row = channelTableWidget->rowCount();
            channelTableWidget->setRowCount(row + 1);
            channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
            channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
            QTableWidgetItem* item = new QTableWidgetItem(channel->getDeviceName());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            channelTableWidget->setItem(row, 1, item);
        }
        channelTableWidget->blockSignals(false);
        channelTableWidget->setFocus(Qt::MouseFocusReason);
        channelTableWidget->setRangeSelected(QTableWidgetSelectionRange(newRow, 0, newRow, 1), true);
    }
}

bool sortByBottomRow(const QTableWidgetSelectionRange range1, const QTableWidgetSelectionRange range2)
{
    return range1.bottomRow() > range2.bottomRow();
}

void OptionsDialog::moveDown()
{
    int newRow = channels.size();
    QList<QTableWidgetSelectionRange> selections = channelTableWidget->selectedRanges();
    qSort(selections.begin(), selections.end(), sortByBottomRow);
    foreach (QTableWidgetSelectionRange selection, selections) {
        for (int i = selection.bottomRow(); i >= selection.topRow(); i--) {
            newRow = i + 1;
            if (newRow < channels.size())
                channels.move(i, newRow);
            else
                break;
        }
    }
    if (newRow < channels.size()) {
        channelTableWidget->clearContents();
        channelTableWidget->setRowCount(0);
        channelTableWidget->blockSignals(true);
        foreach (ChannelInfo* channel, channels) {
            int row = channelTableWidget->rowCount();
            channelTableWidget->setRowCount(row + 1);
            channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
            channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
            QTableWidgetItem* item = new QTableWidgetItem(channel->getDeviceName());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            channelTableWidget->setItem(row, 1, item);
        }
        channelTableWidget->blockSignals(false);
        channelTableWidget->setFocus(Qt::MouseFocusReason);
        channelTableWidget->setRangeSelected(QTableWidgetSelectionRange(newRow, 0, newRow, 1), true);
    }
}

void OptionsDialog::removeMenuPopup()
{
    removeMenu->popup(removeChannel->mapToGlobal(QPoint(0, removeChannel->height())));
}

bool isInChannelList(ChannelInfo* channel, QList<ChannelInfo*> list)
{
    for (int i = 0; i < list.size(); i++)
        if (list[i] == channel)
            return true;
    return false;
}

void OptionsDialog::removeChannels()
{ // O(n^2)
    QList<ChannelInfo*> pointerList;
    QList<QTableWidgetSelectionRange> selections = channelTableWidget->selectedRanges();
    foreach (QTableWidgetSelectionRange selection, selections) {
        for (int i = selection.topRow(); i <= selection.bottomRow(); i++)
            pointerList.push_back(channels.at(i));
    }

    QList<ChannelInfo*> newList;
    for (int i = 0; i < channels.size(); i++)
        if (!isInChannelList(channels[i], pointerList))
            newList.push_back(channels[i]->copy());

    foreach (ChannelInfo* channel, channels)
        delete channel;
    channels.clear();

    channels = newList;

    channelTableWidget->clearContents();
    channelTableWidget->setRowCount(0);
    channelTableWidget->blockSignals(true);
    foreach (ChannelInfo* channel, channels) {
        int row = channelTableWidget->rowCount();
        channelTableWidget->setRowCount(row + 1);
        channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
        QTableWidgetItem* item = new QTableWidgetItem(channel->getDeviceName());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        channelTableWidget->setItem(row, 1, item);
    }
    channelTableWidget->blockSignals(false);
    channelTableWidget->setFocus(Qt::MouseFocusReason);
}

void OptionsDialog::clearChannelList()
{
    foreach (ChannelInfo* channel, channels)
        delete channel;
    channels.clear();
    channelTableWidget->clearContents();
    channelTableWidget->setRowCount(0);
}

void OptionsDialog::initNetworkTab()
{
    QLabel* label1 = new QLabel("Server Listen Port", networkTab);
    QLabel* label2 = new QLabel("Server Stream Port", networkTab);
    QLabel* label3 = new QLabel("Server Password", networkTab);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addWidget(label1, 0, 0, 1, 1);
    listenPortLineEdit = new QLineEdit(networkTab);
    gridLayout->addWidget(listenPortLineEdit, 0, 1, 1, 1);

    gridLayout->addWidget(label2, 1, 0, 1, 1);
    streamPortLineEdit = new QLineEdit(networkTab);
    gridLayout->addWidget(streamPortLineEdit, 1, 1, 1, 1);

    gridLayout->addWidget(label3, 2, 0, 1, 1);
    serverPassLineEdit = new QLineEdit(networkTab);
    gridLayout->addWidget(serverPassLineEdit, 2, 1, 1, 1);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QVBoxLayout* vLayout = new QVBoxLayout(networkTab);
    vLayout->addLayout(gridLayout);
    vLayout->addItem(horizontalSpacer);
}

void OptionsDialog::loadNetworkSettings(TcpCommand* tcpCommand)
{
    listenPortLineEdit->setText(QString::number(tcpCommand->serverListenPort()));
    streamPortLineEdit->setText(QString::number(tcpCommand->serverStreamPort()));
    serverPassLineEdit->setText(tcpCommand->serverPassword());
}

void OptionsDialog::saveNetworkSettings(TcpCommand* tcpCommand)
{
    tcpCommand->setServerListenPort(listenPortLineEdit->text().toInt());
    tcpCommand->setServerStreamPort(streamPortLineEdit->text().toInt());
    tcpCommand->setServerPassword(serverPassLineEdit->text());
}

void OptionsDialog::showOverlayFontDialog()
{
    QString fontFilter = "*ttf";
    QString fontName = QFileDialog::getOpenFileName(this, "Open Font", "", "");
    if (fontName.compare("") != 0) {
        fontOverlay->setText(fontName);
    }
}

void OptionsDialog::showSubtitleFontDialog()
{
    QString fontFilter = "*.ttf";
    QString fontName = QFileDialog::getOpenFileName(this, "Open Font", "", "");
    if (fontName.compare("") != 0) {
        fontSubtitle->setText(fontName);
    }
}

void OptionsDialog::showEntityFontDialog()
{
    QString fontFilter = "*.ttf";
    QString fontName = QFileDialog::getOpenFileName(this, "Open Font", "", "");
    if (fontName.compare("") != 0) {
        fontEntity->setText(fontName);
    }
}

void OptionsDialog::showHeaderFontDialog()
{
    QString fontFilter = "*.ttf";
    QString fontName = QFileDialog::getOpenFileName(this, "Open Font", "", "");
    if (fontName.compare("") != 0) {
        fontHeader->setText(fontName);
    }
}

void OptionsDialog::showFooterFontDialog()
{
    QString fontFilter = "*.ttf";
    QString fontName = QFileDialog::getOpenFileName(this, "Open Font", "", "");
    if (fontName.compare("") != 0) {
        fontFooter->setText(fontName);
    }
}
