#ifndef URLDIALOG_HPP
#define URLDIALOG_HPP

#include <QDialog>
class QLineEdit;

class UrlDialog : public QDialog {
    Q_OBJECT
public:
    UrlDialog(QWidget* parent);
    ~UrlDialog();

    QString retreiveUrl();

protected:
    void setupUI();

private slots:
    void okPressed();
    void cancelPressed();

private:
    QLineEdit* urlLineEdit;
};

#endif
