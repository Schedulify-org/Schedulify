#ifndef COURSE_MODEL_H
#define COURSE_MODEL_H

#include <QAbstractListModel>
#include <vector>
#include <string>

// Course structure definition
struct CourseM {
    QString id;
    QString name;
    QString teacherName;

    CourseM(const QString& id, const QString& name, const QString& teacherName)
            : id(id), name(name), teacherName(teacherName) {}
};

// Qt list model for courses
class CourseModel : public QAbstractListModel {
Q_OBJECT

public:
    // Define roles for the model
    enum CourseRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TeacherRole
    };

    explicit CourseModel(QObject* parent = nullptr);

    // Override methods from QAbstractListModel without default arguments
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // Method to populate the model with sample data
    void populateSampleData();

private:
    std::vector<CourseM> m_courses;
};

#endif // COURSE_MODEL_H