#include "FilterDialog.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

FilterDialog::FilterDialog(QWidget* parent, FilterDialogMode mode, const QString& presetName, QStringList filterPreset, QStringList filterList)
    : QDialog(parent)
{
    QLabel* label1 = new QLabel("Preset Filter Chain", this);
    QLabel* label2 = new QLabel("Available Filters", this);
    QLabel* label3 = new QLabel("Preset Name", this);

    mFilters = new QListWidget(this);
    mFilters->addItems(filterList);
    mFilters->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mSelectedFilters = new QListWidget(this);
    mSelectedFilters->addItems(filterPreset);
    mSelectedFilters->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mPresetName = new QLineEdit(this);
    mPresetName->setText(presetName);
    QPushButton* presetButton;
    if (mode == ADDFILTER)
        presetButton = new QPushButton("Add Preset", this);
    else
        presetButton = new QPushButton("Apply Changes", this);
    connect(presetButton, SIGNAL(pressed()), this, SLOT(presetButtonPressed()));
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, SIGNAL(pressed()), this, SLOT(cancelButtonPressed()));
    QPushButton* moveLeftButton = new QPushButton("<--", this);
    connect(moveLeftButton, SIGNAL(pressed()), this, SLOT(moveLeftButtonPressed()));
    QPushButton* moveRightButton = new QPushButton("-->", this);
    connect(moveRightButton, SIGNAL(pressed()), this, SLOT(moveRightButtonPressed()));

    QVBoxLayout* vDialogLayout = new QVBoxLayout(this);
    QVBoxLayout* vLayout1 = new QVBoxLayout;
    QVBoxLayout* vLayout2 = new QVBoxLayout;
    QVBoxLayout* vLayout3 = new QVBoxLayout;

    QHBoxLayout* hLayout1 = new QHBoxLayout;
    QHBoxLayout* hLayout2 = new QHBoxLayout;
    QHBoxLayout* hLayout3 = new QHBoxLayout;

    QSpacerItem* verticalSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem* verticalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vLayout1->addWidget(label1);
    vLayout1->addWidget(mSelectedFilters);
    vLayout2->addWidget(label2);
    vLayout2->addWidget(mFilters);
    vLayout3->addItem(verticalSpacer);
    vLayout3->addWidget(moveLeftButton);
    vLayout3->addWidget(moveRightButton);
    vLayout3->addItem(verticalSpacer2);

    hLayout1->addWidget(label3);
    hLayout1->addWidget(mPresetName);
    hLayout1->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    hLayout2->addLayout(vLayout1);
    hLayout2->addLayout(vLayout3);
    hLayout2->addLayout(vLayout2);

    hLayout3->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout3->addWidget(presetButton);
    hLayout3->addWidget(cancelButton);

    vDialogLayout->addLayout(hLayout2);
    vDialogLayout->addLayout(hLayout1);
    vDialogLayout->addLayout(hLayout3);

    setWindowTitle("Filter Chain");
    setWindowIcon(QIcon("../resources/Images/title24.png"));
    resize(400, 300);
}

FilterDialog::~FilterDialog()
{
}

void FilterDialog::presetButtonPressed()
{
    accept();
}

void FilterDialog::cancelButtonPressed()
{
    reject();
}

void FilterDialog::moveLeftButtonPressed()
{
    QList<QListWidgetItem*> selectedList = mFilters->selectedItems();
    for (int i = 0; i < selectedList.size(); i++) {
        mSelectedFilters->addItem(selectedList[i]->text());
    }
}

void FilterDialog::moveRightButtonPressed()
{
    QList<QListWidgetItem*> selectedList = mSelectedFilters->selectedItems();
    for (int i = 0; i < selectedList.size(); i++) {
        delete mSelectedFilters->takeItem(mSelectedFilters->row(selectedList[i]));
    }
}

void FilterDialog::retreiveFilterChain(QStringList& filterChain, QString& presetName)
{
    for (int i = 0; i < mSelectedFilters->count(); i++) {
        mSelectedFilters->setCurrentRow(i);
        filterChain << mSelectedFilters->currentItem()->text();
    }
    presetName = mPresetName->text();
}
