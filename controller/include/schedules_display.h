#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "main/model_access.h"
#include "model_interfaces.h"
#include "models/schedule_model.h"

#include <QObject>
#include <QVariant>
#include <QFileDialog>
#include <QDir>
#include <QQuickItem>
#include <QStandardPaths>
#include <QQuickItemGrabResult>
#include <QtQuick/QQuickItem>

enum class fileType {
    PNG,
    CSV
};

class SchedulesDisplayController : public ControllerManager {
Q_OBJECT
    Q_PROPERTY(ScheduleModel* scheduleModel READ scheduleModel CONSTANT)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override;

    void loadScheduleData(const std::vector<InformativeSchedule>& schedules);

    // Properties
    ScheduleModel* scheduleModel() const { return m_scheduleModel; }

    // QML accessible methods
    Q_INVOKABLE void goBack() override;
    Q_INVOKABLE void saveScheduleAsCSV();
    Q_INVOKABLE void printScheduleDirectly();
    Q_INVOKABLE void captureAndSave(QQuickItem* item, const QString& savePath = QString());

    static QString generateFilename(const QString& basePath, int index, fileType type);

signals:
    void screenshotSaved(const QString& path);
    void screenshotFailed();

private:
    ScheduleModel* m_scheduleModel;
    std::vector<InformativeSchedule> m_schedules;
    IModel* modelConnection;
};

#endif // SCHEDULES_DISPLAY_H