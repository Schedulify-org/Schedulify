#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "main/model_factory.h"
#include "model_interfaces.h"

#include <QObject>
#include <QVariant>
#include <QFileDialog>
#include <QDir>
#include <QQuickItem>
#include <QStandardPaths>
#include <QQuickItemGrabResult>

enum class fileType {
    PNG,
    CSV
};

class SchedulesDisplayController : public ControllerManager {
    Q_OBJECT
    Q_PROPERTY(int currentScheduleIndex READ currentScheduleIndex
                            WRITE setCurrentScheduleIndex NOTIFY currentScheduleIndexChanged)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override;

    void loadScheduleData(const vector<InformativeSchedule>& schedules);
    [[nodiscard]] int currentScheduleIndex() const;
    Q_INVOKABLE void setCurrentScheduleIndex(int index);

    Q_INVOKABLE [[nodiscard]] QVariantList getDayItems(int scheduleIndex, int dayIndex) const;
    Q_INVOKABLE [[nodiscard]] static QString getDayName(int dayIndex) ;
    Q_INVOKABLE [[nodiscard]] int getScheduleCount() const;
    Q_INVOKABLE void goBack() override;
    Q_INVOKABLE void saveScheduleAsCSV();
    Q_INVOKABLE void printScheduleDirectly();
    Q_INVOKABLE void captureAndSave(QQuickItem* item, const QString& savePath = QString());


signals:
        void currentScheduleIndexChanged();
        void screenshotSaved(const QString& path);
        void screenshotFailed();
        void scheduleChanged();

private:
    static QString generateFilename(const QString& basePath, int index, fileType type);
    vector<InformativeSchedule> m_schedules;
    int m_currentScheduleIndex;
    IModel* modelConnection;
};

#endif // SCHEDULES_DISPLAY_H