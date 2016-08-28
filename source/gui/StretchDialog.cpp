#include "StretchDialog.hpp"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

StretchDialog::StretchDialog(QWidget* parent, StretchMode mode, const QString& presetName, QVector4D presetValues)
    : QDialog(parent)
{
    mStretchLeftLineEdit = new QLineEdit(QString::number(presetValues.w()), this);
    mStretchRightLineEdit = new QLineEdit(QString::number(presetValues.x()), this);
    mStretchUpLineEdit = new QLineEdit(QString::number(presetValues.y()), this);
    mStretchDownLineEdit = new QLineEdit(QString::number(presetValues.z()), this);
    mPresetNameLineEdit = new QLineEdit(presetName, this);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget(new QLabel("Stretch Left"), 0, 0, 1, 1);
    gridLayout->addWidget(mStretchLeftLineEdit, 0, 1, 1, 1);
    gridLayout->addWidget(new QLabel("Stretch Right"), 0, 3, 1, 1);
    gridLayout->addWidget(mStretchRightLineEdit, 0, 4, 1, 1);
    gridLayout->addWidget(new QLabel("Stretch Up"), 1, 0, 1, 1);
    gridLayout->addWidget(mStretchUpLineEdit, 1, 1, 1, 1);
    gridLayout->addWidget(new QLabel("Stretch Down"), 1, 3, 1, 1);
    gridLayout->addWidget(mStretchDownLineEdit, 1, 4, 1, 1);
    gridLayout->addWidget(new QLabel("Preset Name"), 2, 0, 1, 1);
    gridLayout->addWidget(mPresetNameLineEdit, 2, 1, 1, 1);

    QPushButton* acceptButton;
    if (mode == ADDSTRETCH)
        acceptButton = new QPushButton("Add Preset", this);
    else
        acceptButton = new QPushButton("Apply Changes", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(acceptButtonPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPressed()));

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(acceptButton);
    hLayout->addWidget(cancelButton);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->addLayout(gridLayout);
    vLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vLayout->addLayout(hLayout);

    setWindowTitle("Stretch Preset");
    setWindowIcon(QIcon("../resources/Images/title24.png"));
    resize(400, 200);
}

StretchDialog::~StretchDialog()
{
}

void StretchDialog::acceptButtonPressed()
{
    accept();
}

void StretchDialog::cancelButtonPressed()
{
    reject();
}

void StretchDialog::retreive(QVector4D& stretch, QString& name)
{
    name = mPresetNameLineEdit->text();
    stretch.setW(mStretchLeftLineEdit->text().toInt());
    stretch.setX(mStretchRightLineEdit->text().toInt());
    stretch.setY(mStretchUpLineEdit->text().toInt());
    stretch.setZ(mStretchDownLineEdit->text().toInt());
}
