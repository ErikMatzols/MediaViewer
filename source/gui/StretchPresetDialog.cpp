#include "StretchPresetDialog.hpp"
#include "MainWindow.hpp"
#include "StretchDialog.hpp"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>

StretchPresetDialog::StretchPresetDialog(MainWindow* mainWindow, QStringList presetList)
    : QDialog(mainWindow)
{
    mPresetList = presetList;
    mMainWindow = mainWindow;
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
    setWindowTitle("Stretch Presets");
    setWindowIcon(QIcon("../resources/Images/title24.png"));
    resize(320, 100);
}

StretchPresetDialog::~StretchPresetDialog()
{
}

void StretchPresetDialog::comboBoxIndexActivated(int /*index*/)
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

void StretchPresetDialog::addButtonPressed()
{
    StretchDialog dialog(this, ADDSTRETCH, "", QVector4D(0, 0, 0, 0));
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        QVector4D stretch;
        QString name;
        dialog.retreive(stretch, name);
        if (name.size() == 0)
            name = "Preset" + QString::number(mPresetComboBox->count());
        mPresetComboBox->addItem(name);
        mMainWindow->addStretchPreset(stretch, name, false);
    }
}

void StretchPresetDialog::editButtonPressed()
{
    int index = mPresetComboBox->currentIndex();
    StretchDialog dialog(this, EDITSTRETCH, mPresetComboBox->currentText(), mMainWindow->getStretchPreset(index));
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        QVector4D stretch;
        QString name;
        dialog.retreive(stretch, name);
        if (name.size() == 0)
            name = "Preset" + QString::number(index + 1);
        mPresetComboBox->setItemText(index, name);
        mMainWindow->editStretchPreset(mPresetComboBox->currentIndex(), stretch, name);
    }
}

void StretchPresetDialog::deleteButtonPressed()
{
    int index = mPresetComboBox->currentIndex();
    mMainWindow->deleteStretchPreset(index);
    mPresetComboBox->removeItem(index);
}
