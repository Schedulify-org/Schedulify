#ifndef FILE_INPUT_H
#define FILE_INPUT_H

#include "base_controller.h"

class FileInputController : public BaseController {
Q_OBJECT

public:
    explicit FileInputController(QObject *parent = nullptr);
    ~FileInputController() = default;


public slots:
    void handleUploadAndContinue();
};

#endif //FILE_INPUT_H
