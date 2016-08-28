/*
#include "OptionsAddChannel.hpp"
#include "OptionsDialog.hpp"
#include "WinTvTuner.hpp"
#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QTableWidgetItem>

OptionsAddChannel::OptionsAddChannel(OptionsDialog *optionsDialog)
: QDialog(reinterpret_cast<QWidget*>(optionsDialog))
{
    mTuner = NULL;
    this->optionsDialog = optionsDialog;
    //QPushButton *okButton = new QPushButton("Ok",this);
    //QPushButton *cancelButton = new QPushButton("Cancel",this);
    //connect(okButton,SIGNAL(clicked()),this,SLOT(okPressed()));
    //connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancelPressed()));

    QPushButton *addButton = new QPushButton("Add Channel",this);
    connect(addButton,SIGNAL(clicked()),this,SLOT(addChannel()));
    tuneButton = new QPushButton("AutoTune",this);
    connect(tuneButton,SIGNAL(clicked()),this,SLOT(autotunePressed()));

    QGridLayout *gridLayout = new QGridLayout();
    QLabel *labelDevice = new QLabel("Device");
    comboDevice = new QComboBox(this);
    connect(comboDevice, SIGNAL(activated(int)), this, SLOT(deviceChanged(int)));

    gridLayout->addWidget(labelDevice, 0, 0, 1, 1);
    gridLayout->addWidget(comboDevice, 0, 3, 1, 1);

    QLabel *labelMode = new QLabel("Tuning Mode");
    comboMode = new QComboBox(this);
    gridLayout->addWidget(labelMode, 1, 0, 1, 1);
    gridLayout->addWidget(comboMode, 1, 3, 1, 1);

    QLabel *labelInput = new QLabel("Input");
    comboInput = new QComboBox(this);
    gridLayout->addWidget(labelInput, 2, 0, 1, 1);
    gridLayout->addWidget(comboInput, 2, 3, 1, 1);

    QLabel *labelInputType = new QLabel("Input Type");
    comboInputType = new QComboBox(this);
    gridLayout->addWidget(labelInputType, 3, 0, 1, 1);
    gridLayout->addWidget(comboInputType, 3, 3, 1, 1);

    QLabel *labelCountry = new QLabel("Country Code");
    lineCountry = new QLineEdit(this);
    gridLayout->addWidget(labelCountry, 4, 0, 1, 1);
    gridLayout->addWidget(lineCountry,  4, 3, 1, 1);

    QLabel *labelTuningSpace = new QLabel("Tuning Space");
    lineTuningSpace = new QLineEdit(this);
    gridLayout->addWidget(labelTuningSpace, 5, 0, 1, 1);
    gridLayout->addWidget(lineTuningSpace,  5, 3, 1, 1);


    QLabel *labelChannel = new QLabel("Channel");
    spinChannel = new QSpinBox(this);
    gridLayout->addWidget(labelChannel, 6, 0, 1, 1);
    gridLayout->addWidget(spinChannel, 6, 3, 1, 1);

    QLabel *labelNameChannel = new QLabel("Channel Name");
    lineChannelName = new QLineEdit(this);
    gridLayout->addWidget(labelNameChannel, 7, 0, 1, 1);
    gridLayout->addWidget(lineChannelName, 7, 3, 1, 1);

    //listWidget = new QListWidget(this);
    //listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addLayout(gridLayout);
    //hLayout2->addWidget(listWidget);
    
    QHBoxLayout *hLayout = new QHBoxLayout;
    QSpacerItem *hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayout->addWidget(addButton);
    hLayout->addWidget(tuneButton);
    hLayout->addItem(hSpacer);
    //hLayout->addWidget(okButton);
    //hLayout->addWidget(cancelButton);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout2);
    vLayout->addLayout(hLayout);

    setWindowTitle("Add Channel(s)");
    resize(sizeHint().width(),300);
}

OptionsAddChannel::~OptionsAddChannel()
{

}

void OptionsAddChannel::okPressed()
{
    accept();
    //tuner->closeChannelConfiguration();
}

void OptionsAddChannel::cancelPressed()
{
    reject();
    //tuner->closeChannelConfiguration();
}

int OptionsAddChannel::initializeTuner()
{
    if (!mTuner) {
        mTuner = new WinTvTuner(this);
        connect(mTuner, SIGNAL(channelUpdate(int,bool)),this, SLOT(autoTuneChannelUpdate(int,bool)));
        connect(mTuner, SIGNAL(autoTuneCompleted()),this, SLOT(autoTuneCompleted()));
    }
    
    QDir dir("graphs");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList nameFilter;
    nameFilter << "*.gph";
    dir.setNameFilters(nameFilter);
    QStringList entryList = dir.entryList();
    comboDevice->clear();
    for (int i = 0; i < entryList.size(); i++) {
        QString file = dir.absolutePath() + "/" + entryList[i];
        comboDevice->insertItem(i,entryList[i],QVariant(file));
    }

    return deviceChanged(0);
}

void OptionsAddChannel::autotunePressed()
{
    if (mTuner->isRunning()) {
        mTuner->stopTuner();
    }
    else {
        mTuner->startTuner(AUTOTUNE);
        tuneButton->setText("Cancel AutoTune...");
    }
}

void OptionsAddChannel::getChannelList(QList<QListWidgetItem*>& lst)
{   
    //for(int i = 0; i < listWidget->count(); i++)
    //    lst.append(listWidget->item(i));
}

void OptionsAddChannel::addChannel()
{
    if (!comboDevice->count()) {
        std::cout << "Error: Cannot add channel with no device\n";
        return;
    }
	
    QString channelName = lineChannelName->text();
    if (!channelName.size())
        channelName = "Channel "+QString::number(spinChannel->value());
    ChannelItem *item = new ChannelItem(comboDevice->currentText(),comboMode->currentText(),
                                        comboInput->currentText(),comboInputType->currentText(),
                                        lineCountry->text(),lineTuningSpace->text(),spinChannel->text(),
                                        channelName);
    emit addChannelToTable(item);
}

int OptionsAddChannel::deviceChanged(int index)
{
    mTuner->closeChannelConfiguration();
    comboMode->clear();
    comboInput->clear();
    comboInputType->clear();
    lineCountry->clear();
    lineTuningSpace->clear();

    QStringList modes,inputs,inputTypes;
    QString country,tuningspace;
    int min,max;
    
    int result = mTuner->openChannelConfiguration(comboDevice->itemData(index).toString());
	if (result == -1) {
		std::cout << "Error opening channel configuration\n";
		return -1;
	}

    mTuner->getAvailableModes(modes);
    mTuner->getAvailableInputs(inputs);
    mTuner->getInputType(inputTypes);
    mTuner->getCountryCode(country);
    mTuner->getTuningSpace(tuningspace);
    mTuner->getChannelMinMax(min,max);
    
    comboMode->insertItems(0,modes);
    comboInput->insertItems(0,inputs);
    comboInputType->insertItems(0,inputTypes);
    lineCountry->setText(country);
    lineTuningSpace->setText(tuningspace);
    spinChannel->setRange(min,max);
    spinChannel->setWrapping(true);

	return 0;
}

void OptionsAddChannel::autoTuneChannelUpdate(int channel, bool channelFound)
{
    spinChannel->setValue(channel);
    if (channelFound) {
        QString channelName = "Channel " + QString::number(channel);
        ChannelItem *item = new ChannelItem(comboDevice->currentText(),comboMode->currentText(),
            comboInput->currentText(),comboInputType->currentText(),
            lineCountry->text(),lineTuningSpace->text(),QString::number(channel),
            channelName);
        emit addChannelToTable(item);
    }
}

void OptionsAddChannel::autoTuneCompleted()
{
    tuneButton->setText("AutoTune");
}
*/
