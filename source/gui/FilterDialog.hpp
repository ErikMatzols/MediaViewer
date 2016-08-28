#ifndef FILTERDIALOG_HPP
#define FILTERDIALOG_HPP

#include <QDialog>

class QListWidget;
class QLineEdit;

enum FilterDialogMode { ADDFILTER,
    EDITFILTER };

class FilterDialog : public QDialog {
    Q_OBJECT
public:
    FilterDialog(QWidget* parent, FilterDialogMode mode, const QString& presetName, QStringList filterPreset, QStringList filterList);
    ~FilterDialog();

    void retreiveFilterChain(QStringList& filterChain, QString& presetName);

protected:
private slots:
    void presetButtonPressed();
    void cancelButtonPressed();
    void moveLeftButtonPressed();
    void moveRightButtonPressed();

private:
    QListWidget* mFilters;
    QListWidget* mSelectedFilters;
    QLineEdit* mPresetName;
};

#endif
