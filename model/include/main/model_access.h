#ifndef MODEL_FACTORY_H
#define MODEL_FACTORY_H

#include "model_interfaces.h"
#include "main_model.h"

class ModelAccess {
public:
    static IModel* getModel();
};

#endif //MODEL_FACTORY_H
