#include "timerecorder.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

TimeRecorder::TimeRecorder(QObject *parent) : QObject(parent)
{
    filePath = "minesweeper_records.txt";
    loadRecords();
}

void TimeRecorder::addRecord(int seconds, const QString& difficulty)
{
    TimeRecord record;
    record.seconds = seconds;
    record.date = QDateTime::currentDateTime();
    record.difficulty = difficulty;

    records.append(record);
    std::sort(records.begin(), records.end());
    saveRecords();
}

QList<TimeRecord> TimeRecorder::getSortedRecords() const
{
    return records;
}

void TimeRecorder::loadRecords()
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << filePath;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 3) {
            TimeRecord record;
            record.seconds = parts[0].toInt();
            record.date = QDateTime::fromString(parts[1], Qt::ISODate);
            record.difficulty = parts[2];
            records.append(record);
        }
    }

    file.close();
}

void TimeRecorder::saveRecords() const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入文件:" << filePath;
        return;
    }

    QTextStream out(&file);
    for (const auto& record : records) {
        out << record.seconds << ","
            << record.date.toString(Qt::ISODate) << ","
            << record.difficulty << "\n";
    }

    file.close();
}
void TimeRecorder::clearRecords()
{
    records.clear();
    saveRecords();
}
