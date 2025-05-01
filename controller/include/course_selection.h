#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "controller_manager.h"
#include "CourseModel.h"

class CourseSelectionController : public ControllerManager {
Q_OBJECT

    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override = default;
    void initiateCoursesData(const vector<Course>& courses);

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
signals:

    void dataUpdated();
public slots:
    void generateSchedules();

private:
    CourseModel* m_courseModel;
};

#endif //COURSE_SELECTION_H
