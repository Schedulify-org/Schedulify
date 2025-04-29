#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "base_controller.h"

class SchedulesDisplayController : public BaseController {
    Q_OBJECT

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override = default;


public slots:
    void saveSchedule(int scheduleIndex);
    void printSchedule(int scheduleIndex);
};

#endif //SCHEDULES_DISPLAY_H
