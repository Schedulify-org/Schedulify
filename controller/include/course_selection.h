#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "base_controller.h"
#include "CourseModel.h"

class CourseSelectionController : public BaseController {
Q_OBJECT
    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)
    Q_INVOKABLE void initialize();

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override = default;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }

public slots:
    void generateSchedules();

private:
    CourseModel* m_courseModel;
    void initiateCoursesData();
};

#endif //COURSE_SELECTION_H
