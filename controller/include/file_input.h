#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#include "controller_manager.h"
#include "main_include.h"
#include "main/main_model.h"
#include "course_selection.h"
#include "logger.h"

#include <QFileDialog>
#include <QStandardPaths>

class FileInputController : public ControllerManager {
Q_OBJECT

public:
    explicit FileInputController(QObject *parent = nullptr);
    ~FileInputController() override = default;

public slots:
    void handleUploadAndContinue();
    void handleFileSelected(const QString &filePath);

signals:
    void invalidFileFormat();
    void errorMessage(const QString &message);
    void fileSelected(bool hasFile);
    void fileNameChanged(const QString &fileName);

public:
    Q_INVOKABLE void loadFile();

private:
    QString selectedFilePath;
};

#endif // FILE_INPUT_H
