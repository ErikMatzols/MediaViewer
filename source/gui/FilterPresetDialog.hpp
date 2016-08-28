#ifndef FILTERPRESETDIALOG_HPP
#define FILTERPRESETDIALOG_HPP

#include <QDialog>

class MainWindow;
class QComboBox;
class QPushButton;

enum FilterPresetDialogMode { VIDEOMODE,
    AUDIOMODE };

class FilterPresetDialog : public QDialog {
    Q_OBJECT
public:
    FilterPresetDialog(MainWindow* mainWindow, FilterPresetDialogMode mode, QStringList presetList, QStringList filterList);
    ~FilterPresetDialog();

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

    QStringList mFilterList;
    QStringList mPresetList;
    MainWindow* mMainWindow;
    FilterPresetDialogMode mMode;
};

#endif
