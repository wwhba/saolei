#ifndef TIMERECORDER_H
#define TIMERECORDER_H
#include <QObject>
#include <QList>
#include <QString>
#include <QDateTime>

struct TimeRecord {
    int seconds;
    QDateTime date;
    QString difficulty;

    bool operator<(const TimeRecord& other) const {
        return seconds < other.seconds;
    }
};

class TimeRecorder : public QObject
{
    Q_OBJECT
public:
    explicit TimeRecorder(QObject *parent = nullptr);

    void addRecord(int seconds, const QString& difficulty);
    QList<TimeRecord> getSortedRecords() const;
    void saveRecords() const;
    void clearRecords();
private:
    QList<TimeRecord> records;
    QString filePath;

    void loadRecords();
};
#endif // TIMERECORDER_H
