#ifndef MODEL_FACTORY_H
#define MODEL_FACTORY_H

#include "model_interfaces.h"

class ModelFactory {
public:
    static IModel* createModel();
};

#endif //MODEL_FACTORY_H
