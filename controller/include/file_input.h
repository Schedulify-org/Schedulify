#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#include "controller_manager.h"
#include "main_include.h"
#include "main/main_model.h"
#include "course_selection.h"

class FileInputController : public ControllerManager {
Q_OBJECT

public:
    explicit FileInputController(QObject *parent = nullptr);
    ~FileInputController() override = default;

public slots:
    void handleUploadAndContinue();

public:
    Q_INVOKABLE void loadFile(const QString &filePath);
};

#endif // FILE_INPUT_H
