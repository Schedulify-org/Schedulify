#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#include "base_controller.h"
#include "main_include.h"
#include "main/main_model.h"

class FileInputController : public BaseController {
Q_OBJECT

public:
    explicit FileInputController(QObject *parent = nullptr);
    ~FileInputController() override = default;


public slots:
    void handleUploadAndContinue();
};

#endif //FILE_INPUT_H
