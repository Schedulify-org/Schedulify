#include "../include/CourseModel.h"

CourseModel::CourseModel(QObject* parent)
        : QAbstractListModel(parent){}

int CourseModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_courses.size());
}

QVariant CourseModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_courses.size()))
        return QVariant();

    const Course& course = m_courses[index.row()];

    switch (role) {
        case IdRole:
            return course.id;
        case NameRole:
            return course.name;
        case TeacherRole:
            return course.teacherName;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> CourseModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "courseId";
    roles[NameRole] = "courseName";
    roles[TeacherRole] = "teacherName";
    return roles;
}

void CourseModel::populateSampleData()
{
    // Clear existing data
    beginResetModel();
    m_courses.clear();

    // Add 20 sample courses
    m_courses.push_back(Course("CS101", "Introduction to Computer Science", "Dr. Smith"));
    m_courses.push_back(Course("CS201", "Data Structures", "Prof. Johnson"));
    m_courses.push_back(Course("MATH101", "Calculus I", "Dr. Williams"));
    m_courses.push_back(Course("PHYS201", "Physics for Scientists", "Prof. Garcia"));
    m_courses.push_back(Course("ENG103", "Technical Writing", "Dr. Taylor"));
    m_courses.push_back(Course("BIO110", "Introduction to Biology", "Prof. Martinez"));
    m_courses.push_back(Course("CHEM101", "General Chemistry", "Dr. Robinson"));
    m_courses.push_back(Course("HIST202", "Modern World History", "Prof. Lee"));
    m_courses.push_back(Course("CS310", "Algorithms", "Dr. Clark"));
    m_courses.push_back(Course("PSYCH101", "Introduction to Psychology", "Prof. Adams"));
    m_courses.push_back(Course("ECON201", "Microeconomics", "Dr. Wilson"));
    m_courses.push_back(Course("ART105", "Digital Design", "Prof. Thompson"));
    m_courses.push_back(Course("CS405", "Machine Learning", "Dr. Walker"));
    m_courses.push_back(Course("MATH202", "Linear Algebra", "Prof. Hall"));
    m_courses.push_back(Course("BUS101", "Introduction to Business", "Dr. Lopez"));
    m_courses.push_back(Course("MUS110", "Music Theory", "Prof. Young"));
    m_courses.push_back(Course("SOC201", "Social Psychology", "Dr. King"));
    m_courses.push_back(Course("CS440", "Computer Networks", "Prof. Wright"));
    m_courses.push_back(Course("PHIL103", "Ethics", "Dr. Hill"));
    m_courses.push_back(Course("STAT301", "Applied Statistics", "Prof. Scott"));

    endResetModel();
}