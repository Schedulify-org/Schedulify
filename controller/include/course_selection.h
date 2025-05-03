#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "controller_manager.h"
#include "course_model.h"
#include <QStringList>

class CourseSelectionController : public ControllerManager {
Q_OBJECT

    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)
    Q_PROPERTY(CourseModel* selectedCoursesModel READ selectedCoursesModel CONSTANT)
    Q_PROPERTY(CourseModel* filteredCourseModel READ filteredCourseModel CONSTANT)

signals:
    void selectionChanged();

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override = default;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
    [[nodiscard]] CourseModel* selectedCoursesModel() const { return m_selectedCoursesModel; }
    [[nodiscard]] CourseModel* filteredCourseModel() const { return m_filteredCourseModel; }

    void initiateCoursesData(const vector<Course>& courses);


public slots:
    void generateSchedules();
    void toggleCourseSelection(int index);
    void deselectCourse(int index);
    bool isCourseSelected(int index);
    void filterCourses(const QString& searchText);
    void resetFilter();

private:
    CourseModel* m_courseModel;
    CourseModel* m_selectedCoursesModel;
    CourseModel* m_filteredCourseModel;
    vector<Course> allCourses;
    vector<Course> selectedCourses;
    vector<Course> filteredCourses;
    vector<int> selectedIndices;
    vector<int> filteredIndicesMap; // Maps filtered index to original index
};

#endif //COURSE_SELECTION_H