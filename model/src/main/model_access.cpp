#include "main/model_access.h"
#include "main/main_model.h"

IModel* ModelAccess::getModel() {
    return &Model::getInstance();
}