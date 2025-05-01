#ifndef COURSE_MODEL_H
#define COURSE_MODEL_H

#include "main_include.h"

#include <QAbstractListModel>
#include <utility>
#include <vector>
#include <string>

// Course structure definition
struct CourseM {
    QString id;
    QString name;
    QString teacherName;

    CourseM(QString  id, QString  name, QString  teacherName)
            : id(std::move(id)), name(std::move(name)), teacherName(std::move(teacherName)) {}
};

// Qt list model for courses
class CourseModel : public QAbstractListModel {
Q_OBJECT

public:
    // Define roles for the model
    enum CourseRoles {
        CourseIdRole = Qt::UserRole + 1,
        CourseNameRole,
        TeacherNameRole,
        IsSelectedRole
    };

    explicit CourseModel(QObject* parent = nullptr);

    // Override methods from QAbstractListModel without default arguments
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // Method to populate the model with sample data
    void populateCoursesData(const vector<Course>& courses);

private:
    vector<CourseM> m_courses;
};

#endif // COURSE_MODEL_H