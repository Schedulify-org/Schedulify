#include "CourseValidator.h"

CourseValidator::CourseValidator(IModel* modelConn, const std::vector<Course>& courses, QObject* parent)
        : QObject(parent),
          modelConnection(modelConn),
          courses(courses),
          shouldCancel(false) {
}

CourseValidator::~CourseValidator() {
    cancelValidation();
}

void CourseValidator::validateCourses() {
    QMutexLocker locker(&mutex);

    shouldCancel = false;
    locker.unlock(); // Release lock for the actual work

    std::vector<string>* errorsPtr = nullptr;

    try {
        // Emit initial progress
        emit validationProgress(0);

        // Check cancellation before starting
        if (checkCancellation()) {
            Logger::get().logInfo("Validation cancelled before starting");
            return;
        }

        errorsPtr = static_cast<std::vector<string>*>
        (modelConnection->executeOperation(ModelOperation::VALIDATE_COURSES, &courses, ""));

        // Check cancellation after operation
        if (checkCancellation()) {
            Logger::get().logInfo("Validation was cancelled");
            // Clean up the errors if validation was cancelled
            if (errorsPtr) {
                delete errorsPtr;
                errorsPtr = nullptr;
            }
            return;
        }

        if (errorsPtr) {
            emit validationProgress(100);
            emit coursesValidated(errorsPtr); // Transfer ownership to receiver
        } else {
            Logger::get().logError("Received null validation result");
            auto* emptyErrors = new std::vector<string>();
            emit coursesValidated(emptyErrors);
        }

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during validation: " + std::string(e.what()));

        // Clean up on exception
        if (errorsPtr) {
            delete errorsPtr;
            errorsPtr = nullptr;
        }

        // Check if we should still emit or if cancelled
        if (!checkCancellation()) {
            auto* emptyErrors = new std::vector<string>();
            emptyErrors->push_back("Validation failed due to exception: " + std::string(e.what()));
            emit coursesValidated(emptyErrors);
        }
    } catch (...) {
        Logger::get().logError("Unknown exception during validation");

        // Clean up on exception
        if (errorsPtr) {
            delete errorsPtr;
            errorsPtr = nullptr;
        }

        // Check if we should still emit or if cancelled
        if (!checkCancellation()) {
            auto* emptyErrors = new std::vector<string>();
            emptyErrors->push_back("Validation failed due to unknown exception");
            emit coursesValidated(emptyErrors);
        }
    }
}

void CourseValidator::cancelValidation() {
    QMutexLocker locker(&mutex);
    shouldCancel = true;
}

bool CourseValidator::checkCancellation() {
    QMutexLocker locker(&mutex);
    return shouldCancel;
}