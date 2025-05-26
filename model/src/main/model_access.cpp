#include "model_access.h"

IModel* ModelAccess::getModel() {
    return &Model::getInstance();
}