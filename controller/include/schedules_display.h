#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "model_access.h"
#include "model_interfaces.h"
#include "schedule_model.h"

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
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

    // Sorting methods
    Q_INVOKABLE void applySorting(const QVariantMap& sortData);
    Q_INVOKABLE void clearSorting();

    static QString generateFilename(const QString& basePath, int index, fileType type);

    signals:
        void schedulesSorted(int totalCount);
        void screenshotSaved(const QString& path);
        void screenshotFailed();

private:
    std::vector<InformativeSchedule> m_schedules;
    ScheduleModel* m_scheduleModel;
    IModel* modelConnection;
    QMap<QString, QString> m_sortKeyMap;


    // Track current sort state for optimization
    QString m_currentSortField;
    bool m_currentSortAscending = true;
};

#endif // SCHEDULES_DISPLAY_H