#ifndef PRINT_SCHEDULE_H
#define PRINT_SCHEDULE_H

#include <QString>
#include <QFileDialog>
#include <QPrintDialog>
#include "../main/main_model.h"

// Function to save a schedule to PDF format
void saveToPDF(const InformativeSchedule& schedule, const QString& filePath);

// Function to print a schedule directly to a printer
void printSelectedSchedule(const InformativeSchedule& schedule);

#endif // PRINT_SCHEDULE_H