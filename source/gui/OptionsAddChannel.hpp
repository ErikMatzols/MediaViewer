/*
#ifndef OPTIONSADDCHANNEL_HPP
#define OPTIONSADDCHANNEL_HPP

#include <QDialog>
#include <QList>
class TunerInterface;
class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class OptionsDialog;
class QSpinBox;
class ChannelItem;

class OptionsAddChannel : public QDialog
{
    Q_OBJECT
public:
    OptionsAddChannel(OptionsDialog *optionsDialog);
    ~OptionsAddChannel();

    int initializeTuner();
    void getChannelList(QList<QListWidgetItem*>& lst);

signals:
    void addChannelToTable(ChannelItem *item);

public slots:
    void okPressed();
    void cancelPressed();
    void autotunePressed();
    void addChannel();

    int deviceChanged(int index);

    void autoTuneChannelUpdate(int channel, bool found);
    void autoTuneCompleted();

protected:

private:
    OptionsDialog *optionsDialog;
    TunerInterface *mTuner;
    QListWidget* listWidget;
    QComboBox *comboDevice;
    QComboBox *comboMode;
    QComboBox *comboInput;
    QComboBox *comboInputType;
    QLineEdit *lineCountry;
    QLineEdit *lineTuningSpace;
    QSpinBox  *spinChannel;
    QLineEdit *lineChannelName;

    QPushButton *tuneButton;

};

#endif
*/