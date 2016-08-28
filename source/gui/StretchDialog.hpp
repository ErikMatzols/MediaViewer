#ifndef STRETCHDIALOG_HPP
#define STRETCHDIALOG_HPP

#include <QDialog>
#include <QVector4D>
class QLineEdit;
enum StretchMode { ADDSTRETCH,
    EDITSTRETCH };

class StretchDialog : public QDialog {
    Q_OBJECT
public:
    StretchDialog(QWidget* parent, StretchMode mode, const QString& presetName, QVector4D presetValues);
    ~StretchDialog();

    void retreive(QVector4D& stretch, QString& name);

private slots:
    void acceptButtonPressed();
    void cancelButtonPressed();

protected:
private:
    QLineEdit* mStretchUpLineEdit;
    QLineEdit* mStretchDownLineEdit;
    QLineEdit* mStretchLeftLineEdit;
    QLineEdit* mStretchRightLineEdit;
    QLineEdit* mPresetNameLineEdit;
};

#endif
