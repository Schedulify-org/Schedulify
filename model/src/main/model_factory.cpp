#include "main/model_factory.h"
#include "main/main_model.h"

IModel* ModelFactory::createModel() {
    return new Model();
}