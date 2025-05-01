#include "course_model.h"

CourseModel::CourseModel(QObject* parent)
        : QAbstractListModel(parent){}

int CourseModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_courses.size());
}

QVariant CourseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_courses.size()))
        return {};

    const CourseM& course = m_courses.at(index.row());

    switch (role) {
        case CourseIdRole:
            return course.id;
        case CourseNameRole:
            return course.name;
        case TeacherNameRole:
            return course.teacherName;
        default:
            return {};
    }
}

QHash<int, QByteArray> CourseModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CourseIdRole] = "courseId";
    roles[CourseNameRole] = "courseName";
    roles[TeacherNameRole] = "teacherName";
    roles[IsSelectedRole] = "isSelected";
    return roles;
}

void CourseModel::populateCoursesData(const vector<Course>& courses)
{
    // Clear existing data
    beginResetModel();
    m_courses.clear();

    for (const Course& course : courses) {
        m_courses.emplace_back(
                QString::fromStdString(course.raw_id),
                QString::fromStdString(course.name),
                QString::fromStdString(course.teacher)
        );
    }
    endResetModel();
}