#include "FilterPresetDialog.hpp"
#include "FilterDialog.hpp"
#include "MainWindow.hpp"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>

FilterPresetDialog::FilterPresetDialog(MainWindow* mainWindow, FilterPresetDialogMode mode, QStringList presetList, QStringList filterList)
    : QDialog(mainWindow)
{
    mPresetList = presetList;
    mFilterList = filterList;
    mMainWindow = mainWindow;
    mMode = mode;
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QGridLayout* gridLayout = new QGridLayout();

    mAddPresetButton = new QPushButton("Add", this);
    connect(mAddPresetButton, SIGNAL(pressed()), this, SLOT(addButtonPressed()));
    mEditPresetButton = new QPushButton("Edit", this);
    connect(mEditPresetButton, SIGNAL(pressed()), this, SLOT(editButtonPressed()));
    mDeletePresetButton = new QPushButton("Delete", this);
    connect(mDeletePresetButton, SIGNAL(pressed()), this, SLOT(deleteButtonPressed()));
    mPresetComboBox = new QComboBox(this);
    connect(mPresetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxIndexActivated(int)));
    mPresetComboBox->addItems(presetList);

    gridLayout->addWidget(mPresetComboBox, 0, 0, 1, 2);
    gridLayout->addWidget(mAddPresetButton, 0, 3, 1, 1);
    gridLayout->addWidget(mEditPresetButton, 0, 4, 1, 1);
    gridLayout->addWidget(mDeletePresetButton, 0, 5, 1, 1);

    vLayout->addLayout(gridLayout);
    if (mMode == VIDEOMODE)
        setWindowTitle("Video Filter Presets");
    else if (mMode == AUDIOMODE)
        setWindowTitle("Audio Filter Presets");
    setWindowIcon(QIcon("../resources/Images/title24.png"));
    resize(320, 100);
}

FilterPresetDialog::~FilterPresetDialog()
{
}

void FilterPresetDialog::comboBoxIndexActivated(int /*index*/)
{
    if (mPresetComboBox->currentText().compare("Off") == 0) {
        mAddPresetButton->setEnabled(true);
        mEditPresetButton->setEnabled(false);
        mDeletePresetButton->setEnabled(false);
    } else {
        mAddPresetButton->setEnabled(true);
        mEditPresetButton->setEnabled(true);
        mDeletePresetButton->setEnabled(true);
    }
}

void FilterPresetDialog::addButtonPressed()
{
    FilterDialog dialog(this, ADDFILTER, "", QStringList(), mFilterList);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        QStringList list;
        QString presetName;
        dialog.retreiveFilterChain(list, presetName);
        if (presetName.size() == 0)
            presetName = "Preset" + QString::number(mPresetComboBox->count());
        mPresetComboBox->addItem(presetName);
        if (mMode == VIDEOMODE)
            mMainWindow->addVideoFilterPreset(list, presetName, false);
        else if (mMode == AUDIOMODE)
            mMainWindow->addAudioFilterPreset(list, presetName, false);
    }
}

void FilterPresetDialog::editButtonPressed()
{
    int index = mPresetComboBox->currentIndex();
    QStringList lst = mMode == VIDEOMODE ? mMainWindow->getVideoFilterPreset(index) : mMainWindow->getAudioFilterPreset(index);
    FilterDialog dialog(this, EDITFILTER, mPresetComboBox->currentText(), lst, mFilterList);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        QStringList list;
        QString presetName;
        dialog.retreiveFilterChain(list, presetName);
        if (presetName.size() == 0)
            presetName = "Preset" + QString::number(index + 1);
        mPresetComboBox->setItemText(index, presetName);
        mMode == VIDEOMODE ? mMainWindow->editVideoFilterPreset(index, list, presetName, false) : mMainWindow->editAudioFilterPreset(index, list, presetName, false);
    }
}

void FilterPresetDialog::deleteButtonPressed()
{
    int index = mPresetComboBox->currentIndex();
    mMode == VIDEOMODE ? mMainWindow->deleteVideoFilterPreset(index) : mMainWindow->deleteAudioFilterPreset(index);
    mPresetComboBox->removeItem(index);
}
