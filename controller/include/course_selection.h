#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "controller_manager.h"
#include "course_model.h"

class CourseSelectionController : public ControllerManager {
Q_OBJECT

    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)
    Q_PROPERTY(CourseModel* selectedCoursesModel READ selectedCoursesModel CONSTANT)

signals:
    void selectionChanged();

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override = default;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
    [[nodiscard]] CourseModel* selectedCoursesModel() const { return m_selectedCoursesModel; }

    void initiateCoursesData(const vector<Course>& courses);


public slots:
    void generateSchedules();
    void toggleCourseSelection(int index);
    void deselectCourse(int index);
    bool isCourseSelected(int index);

private:
    CourseModel* m_courseModel;
    CourseModel* m_selectedCoursesModel;
    vector<Course> allCourses;
    vector<Course> selectedCourses;
    vector<int> selectedIndices;
};

#endif //COURSE_SELECTION_H
