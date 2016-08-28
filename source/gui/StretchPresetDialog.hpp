#ifndef STRETCHPRESETDIALOG_HPP
#define STRETCHPRESETDIALOG_HPP

#include <QDialog>

class MainWindow;
class QComboBox;
class QPushButton;
class StretchDialog;

class StretchPresetDialog : public QDialog {
    Q_OBJECT
public:
    StretchPresetDialog(MainWindow* mainWindow, QStringList presetList);
    ~StretchPresetDialog();

protected:
private slots:
    void comboBoxIndexActivated(int index);

    void addButtonPressed();
    void editButtonPressed();
    void deleteButtonPressed();

private:
    QComboBox* mPresetComboBox;
    QPushButton* mAddPresetButton;
    QPushButton* mEditPresetButton;
    QPushButton* mDeletePresetButton;

    QStringList mPresetList;
    MainWindow* mMainWindow;
};

#endif
