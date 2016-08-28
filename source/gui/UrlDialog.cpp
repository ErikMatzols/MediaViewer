#include "UrlDialog.hpp"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

UrlDialog::UrlDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Open Url");
}

UrlDialog::~UrlDialog()
{
}

void UrlDialog::setupUI()
{
    urlLineEdit = new QLineEdit(this);

    QPushButton* okButton = new QPushButton("Ok", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(urlLineEdit);
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);
}

QString UrlDialog::retreiveUrl()
{
    return urlLineEdit->text();
}

void UrlDialog::okPressed()
{
    accept();
}

void UrlDialog::cancelPressed()
{
    reject();
}
