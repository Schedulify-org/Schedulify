#pragma once
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <vector>
#include "model_access.h"
#include "model_interfaces.h"

class CourseValidator : public QObject {
Q_OBJECT

public:
    CourseValidator(IModel* modelConn, const std::vector<Course>& courses, QObject* parent = nullptr);
    ~CourseValidator();

public slots:
    void validateCourses();
    void cancelValidation();

signals:
    void coursesValidated(std::vector<string>* errors);
    void validationProgress(int percentage);

private:
    IModel* modelConnection;
    std::vector<Course> courses;
    volatile bool shouldCancel;
    mutable QMutex mutex;

    bool checkCancellation();
};