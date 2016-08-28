#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "WinTuner.hpp"
#include <QTimer>
#include <QMenu>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QSpinBox>
#include <QDir>
#include "OptionsDialog.hpp"
#include "VideoRenderer.hpp"
#include "WinTunerScanRequest.hpp"

class WinTunerScanner : public QDialog
{
    Q_OBJECT
public:
    WinTunerScanner(OptionsDialog *parent)
        : QDialog(parent)
    {
        optionsDialog = parent;
           
        mPreviewWindow = NULL; //new VideoRenderer(this);
        setupUI();
        winTuner = new WinTuner(this, mPreviewWindow);
        connect(winTuner, SIGNAL(graphRunning()), this, SLOT(graphRunning()));
        connect(winTuner, SIGNAL(signalReceived(long)), this, SLOT(signalReceived(long)));
        connect(winTuner, SIGNAL(analogChannelChanged(int)), this, SLOT(analogChannelChanged(int)));
        winTunerScanRequest = new WinTunerScanRequest(this);
        connect(winTunerScanRequest, SIGNAL(channelsFound(QList<ChannelInfo*>)), this, SLOT(channelsFound(QList<ChannelInfo*>)));
        connect(winTunerScanRequest, SIGNAL(scanRequestFinished()), this, SLOT(scanRequestFinished()));
        
        QDir graphDirectory("../graphs");
        QStringList files = graphDirectory.entryList(QDir::Files | QDir::NoDotAndDotDot);
        deviceCombo->addItems(files);

        resize(500, 320);
    }

    ~WinTunerScanner()
    {
        if (winTuner->isRunning())
            winTuner->stopDVB();
        foreach(ChannelInfo *channel, scannedChannelList)
            delete channel;
        scannedChannelList.clear();
    }

    void setupUI()
    {
        QGroupBox *upperWidget = new QGroupBox("Scanned Channel List", this);
        QGroupBox *middleWidget = new QGroupBox("Current Device", this);
        QGroupBox *lowerWidget = new QGroupBox("Tuning Parameters", this);

        channelTableWidget = new QTableWidget(upperWidget);
        channelTableWidget->setAlternatingRowColors(true);
        channelTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        channelTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        channelTableWidget->horizontalHeader()->setStretchLastSection(true);
        channelTableWidget->verticalHeader()->setDefaultSectionSize(20);
        channelTableWidget->setColumnCount(2);
        channelTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Channel"));
        channelTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Device"));
        connect(channelTableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(ChannelItemChanged(QTableWidgetItem*)));

        sendChannelButton = new QPushButton("Send", upperWidget);
        connect(sendChannelButton, SIGNAL(pressed()), this, SLOT(sendChannelButtonPressed()));
        removeChannelButton = new QPushButton("Remove", upperWidget);
        connect(removeChannelButton, SIGNAL(pressed()), this, SLOT(removeChannelButtonPressed()));
        QPushButton *channelInfoButton = new QPushButton("Info", upperWidget);
        connect(channelInfoButton, SIGNAL(pressed()), this, SLOT(channelInfo()));
        
        QPushButton *previewChannelButton = new QPushButton("Preview", upperWidget);
        connect(previewChannelButton, SIGNAL(pressed()), this, SLOT(previewSelectedChannel()));

        sendChannelMenu = new QMenu(sendChannelButton);
        sendChannelMenu->addAction("Selected Channel(s)", this, SLOT(sendSelectedChannels()));
        sendChannelMenu->addAction("All Channels", this, SLOT(sendAllChannels()));
        
        removeChannelMenu = new QMenu(removeChannelButton);
        removeChannelMenu->addAction("Selected Channel(s)", this, SLOT(removeSelectedChannels()));
        removeChannelMenu->addAction("All Channels", this, SLOT(removeAllChannels()));

        QHBoxLayout *hChannelButtonLayout = new QHBoxLayout;
        hChannelButtonLayout->addWidget(sendChannelButton);
        hChannelButtonLayout->addWidget(removeChannelButton);
        hChannelButtonLayout->addWidget(channelInfoButton);
        hChannelButtonLayout->addWidget(previewChannelButton);
        hChannelButtonLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

        QVBoxLayout *vUpperWidgetLayout = new QVBoxLayout(upperWidget);
        vUpperWidgetLayout->addWidget(channelTableWidget);
        vUpperWidgetLayout->addLayout(hChannelButtonLayout);

        analogWidget = new QWidget(lowerWidget);
        dvbtWidget = new QWidget(lowerWidget);
        dvbcWidget = new QWidget(lowerWidget);
        dvbsWidget = new QWidget(lowerWidget);
        
        deviceCombo = new QComboBox(middleWidget);
        deviceCombo->setMaximumWidth(150);
        connect(deviceCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(deviceChanged(const QString&)));
        typeCombo = new QComboBox(middleWidget);
        typeCombo->setMaximumWidth(100);
        connect(typeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(typeChanged(const QString&)));
        
        middleGridLayout = new QGridLayout(middleWidget);
        middleGridLayout->addWidget(new QLabel("Device:", middleWidget), 0, 0, 1, 1);
        middleGridLayout->addWidget(deviceCombo, 0, 1, 1, 1);
        middleGridLayout->addWidget(new QLabel("Type:", middleWidget), 1, 0 , 1 ,1);
        middleGridLayout->addWidget(typeCombo, 1, 1, 1, 1);
        middleGridLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 2, 1, 1);

        tuningModeComboBox = new QComboBox(analogWidget);
        tuningModeComboBox->setMaximumWidth(100);
        inputComboBox = new QComboBox(analogWidget);
        inputComboBox->setMaximumWidth(100);
        inputTypeComboBox = new QComboBox(analogWidget);
        inputTypeComboBox->setMaximumWidth(100);
        countryCodeLineEdit = new QLineEdit(analogWidget);
        countryCodeLineEdit->setMaximumWidth(100);
        tuningSpaceLineEdit = new QLineEdit(analogWidget);
        tuningSpaceLineEdit->setMaximumWidth(100);
        channelSpinBox = new QSpinBox(analogWidget);
        channelSpinBox->setMaximumWidth(50);
        channelNameLineEdit = new QLineEdit(analogWidget);
        channelNameLineEdit->setMaximumWidth(100);

        analogGridLayout = new QGridLayout(analogWidget);
        analogGridLayout->addWidget(new QLabel("Tuning Mode:", analogWidget), 0, 0, 1, 1);
        analogGridLayout->addWidget(tuningModeComboBox, 0, 1, 1 , 1);
        analogGridLayout->addWidget(new QLabel("Input:", analogWidget), 1, 0, 1 ,1);
        analogGridLayout->addWidget(inputComboBox, 1, 1, 1, 1);
        analogGridLayout->addWidget(new QLabel("Input Type:", analogWidget), 2, 0, 1, 1);
        analogGridLayout->addWidget(inputTypeComboBox, 2, 1, 1, 1);
        analogGridLayout->addWidget(new QLabel("Country Code:", analogWidget), 3, 0 , 1, 1);
        analogGridLayout->addWidget(countryCodeLineEdit, 3, 1, 1, 1);
        analogGridLayout->addWidget(new QLabel("Tuning Space:", analogWidget), 4, 0 ,1 , 1);
        analogGridLayout->addWidget(tuningSpaceLineEdit, 4, 1, 1, 1);
        analogGridLayout->addWidget(new QLabel("Channel:", analogWidget), 5, 0, 1 , 1);
        analogGridLayout->addWidget(channelSpinBox, 5, 1, 1, 1);
        analogGridLayout->addWidget(new QLabel("Channel Name:", analogWidget), 6, 0, 1, 1);
        analogGridLayout->addWidget(channelNameLineEdit, 6, 1, 1, 1);
        analogGridLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 6, 2, 1, 1);
        
        dvbtFreqLineEdit = new QLineEdit(dvbtWidget);
        dvbtFreqLineEdit->setMaximumWidth(50);
        dvbtBandwidthComboBox = new QComboBox(dvbtWidget);
        dvbtBandwidthComboBox->addItems(QStringList() << "6" << "7" << "8");

        dvbtGridLayout = new QGridLayout(dvbtWidget);
        dvbtGridLayout->addWidget(new QLabel("Frequency:", dvbtWidget), 0, 0, 1, 1);
        dvbtGridLayout->addWidget(dvbtFreqLineEdit, 0, 1, 1, 1);
        dvbtGridLayout->addWidget(new QLabel("MHz"), 0, 2, 1, 1);
        dvbtGridLayout->addWidget(new QLabel("Bandwidth:", dvbtWidget), 1, 0, 1, 1);
        dvbtGridLayout->addWidget(dvbtBandwidthComboBox, 1, 1, 1, 1);
        dvbtGridLayout->addWidget(new QLabel("MHz"), 1, 2, 1, 1);
        dvbtGridLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 3, 1, 1);

        dvbcFreqLineEdit = new QLineEdit("", dvbcWidget);
        dvbcModulationComboBox = new QComboBox(dvbcWidget);
        dvbcSymbolrateComboBox = new QComboBox(dvbcWidget);
        
        dvbcGridLayout = new QGridLayout(dvbcWidget);
        dvbcGridLayout->addWidget(new QLabel("Frequency:", dvbcWidget), 0, 0, 1, 1);
        dvbcGridLayout->addWidget(dvbcFreqLineEdit, 0, 1, 1, 1);
        dvbcGridLayout->addWidget(new QLabel("Modulation:", dvbcWidget), 1, 0, 1 ,1);
        dvbcGridLayout->addWidget(dvbcModulationComboBox, 1, 1, 1, 1);
        dvbcGridLayout->addWidget(new QLabel("Symbolrate:", dvbcWidget), 2, 0 ,1 ,1);
        dvbcGridLayout->addWidget(dvbcSymbolrateComboBox, 2, 1, 1, 1);
        dvbcGridLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 2, 1, 1);

        dvbsFreqLine = new QLineEdit(dvbsWidget);
        dvbsPolarisationComboBox = new QComboBox(dvbsWidget);
        dvbsSymbolrateComboBox = new QComboBox(dvbsWidget);
        dvbsModulationComboBox = new QComboBox(dvbsWidget);
        dvbsFECCombo = new QComboBox(dvbsWidget);
        
        dvbsGridLayout = new QGridLayout(dvbsWidget);
        dvbsGridLayout->addWidget(new QLabel("Frequency:", dvbsWidget), 0, 0, 1, 1);
        dvbsGridLayout->addWidget(dvbsFreqLine, 0, 1, 1, 1);
        dvbsGridLayout->addWidget(new QLabel("Polarisation:", dvbsWidget), 1, 0, 1, 1);
        dvbsGridLayout->addWidget(dvbsPolarisationComboBox, 1, 1, 1, 1);
        dvbsGridLayout->addWidget(new QLabel("Symbolrate:", dvbsWidget), 2, 0, 1, 1);
        dvbsGridLayout->addWidget(dvbsSymbolrateComboBox, 2, 1, 1, 1);
        dvbsGridLayout->addWidget(new QLabel("Modulation:", dvbsWidget), 3, 0, 1 ,1);
        dvbsGridLayout->addWidget(dvbsModulationComboBox, 3, 1 ,1 ,1);
        dvbsGridLayout->addWidget(new QLabel("FEC:", dvbsWidget), 4, 0, 1 ,1);
        dvbsGridLayout->addWidget(dvbsFECCombo, 4, 1, 1, 1);
        dvbsGridLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 4, 2, 1, 1);
       
        tuneButton = new QPushButton("Scan/Add", lowerWidget);
        connect(tuneButton, SIGNAL(pressed()), this, SLOT(tuneButtonPressed()));
        autotuneButton = new QPushButton("Autotune", lowerWidget);
        connect(autotuneButton, SIGNAL(pressed()), this, SLOT(autotuneButtonPressed()));
        
        filterButton = new QPushButton("Filters", lowerWidget);
        connect(filterButton, SIGNAL(pressed()), this, SLOT(filterButtonPressed()));

        filterMenu = new QMenu(filterButton);
        connect(filterMenu, SIGNAL(triggered(QAction*)), this, SLOT(menuPressed(QAction*)));

        QHBoxLayout *hButtonLayout = new QHBoxLayout;
        hButtonLayout->addWidget(tuneButton);
        hButtonLayout->addWidget(autotuneButton);
        hButtonLayout->addWidget(filterButton);
        hButtonLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        
        QVBoxLayout *vLayoutTuningParameters = new QVBoxLayout;
        vLayoutTuningParameters->addWidget(analogWidget);
        vLayoutTuningParameters->addWidget(dvbtWidget);
        vLayoutTuningParameters->addWidget(dvbcWidget);
        vLayoutTuningParameters->addWidget(dvbsWidget);
        vLayoutTuningParameters->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
        
        //mPreviewWindow->setMinimumWidth(240);
        //mPreviewWindow->setMinimumHeight(200);
        //mPreviewWindow->setMaximumWidth(240);
        //mPreviewWindow->setMaximumHeight(200);
        
        QHBoxLayout *hLayoutTunePreview = new QHBoxLayout;
        hLayoutTunePreview->addLayout(vLayoutTuningParameters);
        //hLayoutTunePreview->addWidget(mPreviewWindow);

        QVBoxLayout *vLayout = new QVBoxLayout(lowerWidget);
        vLayout->addLayout(hLayoutTunePreview);
        vLayout->addLayout(hButtonLayout);

        analogWidget->hide();
        dvbtWidget->hide();
        dvbcWidget->hide();
        dvbsWidget->hide();
        autotuneButton->hide();

        QVBoxLayout *vMainLayout = new QVBoxLayout(this);
        vMainLayout->addWidget(upperWidget);
        vMainLayout->addWidget(middleWidget);
        vMainLayout->addWidget(lowerWidget);
    }

public slots:
    void graphRunning()
    {
        QStringList tuningSpaces = winTuner->queryTuningSpaces();
        QStringList filters = winTuner->queryFilters();
        
        typeCombo->clear();
        typeCombo->addItems(tuningSpaces);
        
        filterMenu->clear();
        foreach(QString filterName, filters) {
            QAction *action = new QAction(filterName, this);
            filterMenu->addAction(action);
        }
    }

    void signalReceived(long /*strength*/)
    {

    }

signals:
    void sendChannels(QList<ChannelInfo*> lst);

protected:
    

private slots:
    void analogChannelChanged(int channel)
    {
        channelSpinBox->setValue(channel);
    }

    void channelInfo()
    {
        QList<QTableWidgetSelectionRange> selectionRanges = channelTableWidget->selectedRanges();
        int firstSelection = selectionRanges.first().topRow();
        ChannelInfo *channel = scannedChannelList.at(firstSelection);
        switch(channel->ID())
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
    }

    void scanRequestFinished()
    {
        tuneButton->setText("Scan/Add");
        autotuneButton->setText("Autotune");
    }

    void channelsFound(QList<ChannelInfo*> channels)
    {
        scannedChannelList.append(channels);
        foreach (ChannelInfo* channel, channels) {
            int row = channelTableWidget->rowCount();
            channelTableWidget->setRowCount(row +1);
            channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row+1)));
            channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
            QTableWidgetItem *item = new QTableWidgetItem(channel->getDeviceName());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            channelTableWidget->setItem(row, 1, item);
        }
    }

    void previewSelectedChannel()
    {
        // TODO: if scanrequest is running stop i!!t
        QList<QTableWidgetSelectionRange> selectionRanges = channelTableWidget->selectedRanges();
        int firstSelection = selectionRanges.first().topRow();
        scannedChannelList.at(firstSelection)->applySettingsToTuner(winTuner);
    }

    void sendAllChannels()
    { 
        QList<ChannelInfo*> allChannels;
        foreach(ChannelInfo* channel, scannedChannelList)
            allChannels.push_back(channel->copy());
        emit sendChannels(allChannels);
    }

    void sendSelectedChannels()
    {
        QList<ChannelInfo*> selectedChannels;
        QList<QTableWidgetSelectionRange> selectionRanges = channelTableWidget->selectedRanges();
        foreach(QTableWidgetSelectionRange selection, selectionRanges) {
            for(int i = selection.topRow(); i <= selection.bottomRow(); i++) {
                selectedChannels.push_back(scannedChannelList.at(i)->copy());
            }
        }
        emit sendChannels(selectedChannels);
    }

    void removeAllChannels()
    {
        foreach(ChannelInfo *channel, scannedChannelList)
            delete channel;
        scannedChannelList.clear();
        channelTableWidget->clearContents();
        channelTableWidget->setRowCount(0);
    }

    void removeSelectedChannels()
    {
        QList<ChannelInfo*> pointerList;
        QList<QTableWidgetSelectionRange> selections = channelTableWidget->selectedRanges();
        foreach (QTableWidgetSelectionRange selection, selections) {
            for (int i = selection.topRow(); i <= selection.bottomRow(); i++)
                pointerList.push_back(scannedChannelList.at(i));
        }

        for (int i = 0; i < scannedChannelList.size(); i++)
            for(int j = 0; j < pointerList.size(); j++)
                if (pointerList[i] == scannedChannelList[i]) {
                    delete scannedChannelList[i];
                    scannedChannelList.removeAt(i);
                }

        channelTableWidget->clearContents();
        channelTableWidget->setRowCount(0);
        channelTableWidget->blockSignals(true);
        foreach (ChannelInfo *channel, scannedChannelList) {
            int row = channelTableWidget->rowCount();
            channelTableWidget->setRowCount(row +1);
            channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row+1)));
            channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
            QTableWidgetItem *item = new QTableWidgetItem(channel->getDeviceName());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            channelTableWidget->setItem(row, 1, item);
        }
        channelTableWidget->blockSignals(false);
        channelTableWidget->setFocus(Qt::MouseFocusReason);
    }

    void ChannelItemChanged(QTableWidgetItem *item)
    {
        if (item->flags() & Qt::ItemIsEditable){
            scannedChannelList[(item->row())]->setChannelName(item->text());
        }
    }

    void deviceChanged(const QString &device)
    {
        // TODO: if scanrequest isrunning stop it!!
        if (winTunerScanRequest->isRunning()) {
            winTunerScanRequest->stopScanRequest();
            tuneButton->setText("Scan/Add");
            autotuneButton->setText("Autotune");
        }

        if (winTuner->isRunning())
            winTuner->stopDVB();
        winTuner->startDVB(device);
    }

    void typeChanged(const QString &type)
    {
        // TODO: if scanrequest isrunning stop it!!
        if (winTunerScanRequest->isRunning()) {
            winTunerScanRequest->stopScanRequest();
            tuneButton->setText("Scan/Add");
            autotuneButton->setText("Autotune");
        }

        bool showWidget[4];
        if (type.compare("Analog") == 0) {
            showWidget[0] = true; showWidget[1] = false; showWidget[2] = false; showWidget[3] = false;
            tuningModeComboBox->clear();
            tuningModeComboBox->addItems(winTuner->getAvailableModes());
            inputComboBox->clear();
            inputComboBox->addItems(winTuner->getAvailableInputs());
            inputTypeComboBox->clear();
            inputTypeComboBox->addItems(winTuner->getInputTypes());
            countryCodeLineEdit->setText(winTuner->getCountryCode());
            tuningSpaceLineEdit->setText(winTuner->getTuningSpace());
            int max, min;
            winTuner->getChannelMinMax(min, max);
            channelSpinBox->setRange(min, max);
        }
        else if (type.compare("ATSC") == 0) {
            showWidget[0] = false; showWidget[1] = true; showWidget[2] = false; showWidget[3] = false;
            autotuneButton->setVisible(true);
        }
        else if (type.compare("DVB-T") == 0 || type.compare("ISDB-T") == 0) {
            showWidget[0] = false; showWidget[1] = true; showWidget[2] = false; showWidget[3] = false;
            autotuneButton->setVisible(true);
        }
        else if(type.compare("DVB-C") == 0) {
            showWidget[0] = false; showWidget[1] = false; showWidget[2] = true; showWidget[3] = false;
        }
        else if (type.compare("DVB-S") == 0) {
            showWidget[0] = false; showWidget[1] = false; showWidget[2] = false; showWidget[3] = true;
        }
        else {
            showWidget[0] = false; showWidget[1] = false; showWidget[2] = false; showWidget[3] = false;
        }
        analogWidget->setVisible(showWidget[0]);
        dvbtWidget->setVisible(showWidget[1]);
        dvbcWidget->setVisible(showWidget[2]);
        dvbsWidget->setVisible(showWidget[3]);
        autotuneButton->setVisible(showWidget[0]);
        tuneButton->setText("Scan/Add");
    }

    void tuneButtonPressed()
    {
        QList<TuneRequestInfo*> tuneRequests;

        if (winTunerScanRequest->isRunning()) {
            winTunerScanRequest->stopScanRequest();
            tuneButton->setText("Scan/Add");
        }

        if (typeCombo->currentText().compare("Analog") == 0) 
        {
            AnalogTuneRequest tuneReq;
            tuneReq.mode = winTuner->modeToEnum(tuningModeComboBox->currentText());
            tuneReq.input = inputComboBox->currentText().toLong();
            tuneReq.inputType = winTuner->inputTypeToEnum(inputTypeComboBox->currentText());
            tuneReq.countryCode = countryCodeLineEdit->text().toLong();
            tuneReq.tuningSpace = tuningSpaceLineEdit->text().toLong();
            tuneReq.channel = channelSpinBox->value();
            QString channelName = channelNameLineEdit->text();
            if (channelName.length() == 0)
                channelName = "Channel" + QString::number(tuneReq.channel);
            
            ChannelInfo *channel = new AnalogChannelInfo(tuneReq, channelName, deviceCombo->currentText());
            int row = channelTableWidget->rowCount();
            channelTableWidget->setRowCount(row +1);
            channelTableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row+1)));
            channelTableWidget->setItem(row, 0, new QTableWidgetItem(channel->getChannelName()));
            QTableWidgetItem *item = new QTableWidgetItem(channel->getDeviceName());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            channelTableWidget->setItem(row, 1, item);
        }
        else if (typeCombo->currentText().compare("DVB-T") == 0 || typeCombo->currentText().compare("ISDB-T") == 0) {
            DVBTTuneRequest *tuneReq = new DVBTTuneRequest;
            tuneReq->frequency = dvbtFreqLineEdit->text().toLong();
            tuneReq->bandwidth = dvbtBandwidthComboBox->currentText().toLong();
            tuneReq->ONID = -1;
            tuneReq->SID = -1;
            tuneReq->TSID = -1;
            tuneRequests.push_back(tuneReq);
            winTunerScanRequest->startScanRequest(winTuner, tuneRequests);
            tuneButton->setText("Abort..");
        }
        else if (typeCombo->currentText().compare("DVB-C") == 0) {
            //DVBCTuneRequest *tuneReq = new DVBCTuneRequest;
            // fill in struct
            //tuneRequests.push_back(tuneReq);
            //winTunerScanRequest->startScanRequest(winTuner, tuneRequests);
            //tuneButton->setText("Abort..");
        }
        else if (typeCombo->currentText().compare("DVB-C") == 0) {
            //DVBSTuneRequest *tuneReq = new DVBSTuneRequest;
            // fill in struct
            //tuneRequests.push_back(tuneReq);
            //winTunerScanRequest->startScanRequest(winTuner, tuneRequests);
            //tuneButton->setText("Abort..");
        }
    }

    void sendChannelButtonPressed()
    {
        sendChannelMenu->popup(sendChannelButton->mapToGlobal(QPoint(0, sendChannelButton->height())));
    }

    void removeChannelButtonPressed()
    {
        removeChannelMenu->popup(removeChannelButton->mapToGlobal(QPoint(0, removeChannelButton->height())));
    }

    void autotuneButtonPressed()
    {
        // TODO disable tuneButton
        if (winTunerScanRequest->isRunning()) {
            winTunerScanRequest->stopScanRequest();
            autotuneButton->setText("Autotune");
        }
        else if (typeCombo->currentText().compare("Analog") == 0) {
            AnalogTuneRequest tuneReq;
            tuneReq.mode = winTuner->modeToEnum(tuningModeComboBox->currentText());
            tuneReq.input = inputComboBox->currentText().toLong();
            tuneReq.inputType = winTuner->inputTypeToEnum(inputTypeComboBox->currentText());
            tuneReq.countryCode = countryCodeLineEdit->text().toLong();
            tuneReq.tuningSpace = tuningSpaceLineEdit->text().toLong();
            QList<TuneRequestInfo*> tuneRequests;
            for (int i = channelSpinBox->minimum(); i <= channelSpinBox->maximum(); i++) {
                AnalogTuneRequest *t = new AnalogTuneRequest;
                *t = tuneReq;
                t->channel = i;
                tuneRequests.push_back(t);
            }
            winTunerScanRequest->startScanRequest(winTuner, tuneRequests);
            autotuneButton->setText("Abort..");
        }
    }
    
    void filterButtonPressed()
    {
        filterMenu->popup(filterButton->mapToGlobal(QPoint(0,filterButton->height())));
    }

    void menuPressed(QAction *action)
    {
        winTuner->showPropertyPage(action->text());
    }

    void timeout()
    {
        if (!winTuner->isRunning())
            winTuner->start();
    }

private:
    OptionsDialog *optionsDialog;
    WinTuner *winTuner;
    WinTunerScanRequest *winTunerScanRequest;
    QTableWidget *channelTableWidget;
    QPushButton *sendChannelButton;
    QMenu *sendChannelMenu;
    QPushButton *removeChannelButton;
    QMenu *removeChannelMenu;
    QList<ChannelInfo*> scannedChannelList;

    QComboBox *deviceCombo;
    QComboBox *typeCombo;

    QWidget *analogWidget;
    QWidget *dvbtWidget;
    QWidget *dvbcWidget;
    QWidget *dvbsWidget;

    QGridLayout *middleGridLayout;
    QGridLayout *analogGridLayout;
    QGridLayout *dvbtGridLayout;
    QGridLayout *dvbcGridLayout;
    QGridLayout *dvbsGridLayout;

    QComboBox *tuningModeComboBox;
    QComboBox *inputComboBox;
    QComboBox *inputTypeComboBox;
    QLineEdit *countryCodeLineEdit;
    QLineEdit *tuningSpaceLineEdit;
    QSpinBox *channelSpinBox;
    QLineEdit *channelNameLineEdit;

    QLineEdit *dvbtFreqLineEdit;
    QComboBox *dvbtBandwidthComboBox;

    QLineEdit *dvbcFreqLineEdit;
    QComboBox *dvbcModulationComboBox;
    QComboBox *dvbcSymbolrateComboBox;

    QLineEdit *dvbsFreqLine;
    QComboBox *dvbsPolarisationComboBox;
    QComboBox *dvbsSymbolrateComboBox;
    QComboBox *dvbsModulationComboBox;
    QComboBox *dvbsFECCombo;

    QPushButton *tuneButton;
    QPushButton *autotuneButton;
    VideoRenderer *mPreviewWindow;

    QMenu *filterMenu;
    QList<QAction*> filterActions;
    QPushButton *filterButton;
};

#endif
