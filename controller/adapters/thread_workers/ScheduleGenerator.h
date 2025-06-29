#pragma once
#include <QObject>
#include <vector>
#include "model_access.h"
#include "model_interfaces.h"

class ScheduleGenerator : public QObject {
Q_OBJECT

public:
    ScheduleGenerator(IModel* modelConn, const std::vector<Course>& courses, QObject* parent = nullptr);

public slots:
    void generateSchedules();

signals:
    void schedulesGenerated(std::vector<InformativeSchedule>* schedules);

private:
    IModel* modelConnection;
    std::vector<Course> selectedCourses;
};