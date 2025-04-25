#ifndef COURSEMODEL_H
#define COURSEMODEL_H

#include <QAbstractListModel>
#include <vector>
#include <string>

// Course structure definition
struct Course {
    QString id;
    QString name;
    QString teacherName;

    Course(const QString& id, const QString& name, const QString& teacherName)
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

    // Required QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Method to populate the model with sample data
    void populateSampleData();

private:
    std::vector<Course> m_courses;
};

#endif // COURSEMODEL_H