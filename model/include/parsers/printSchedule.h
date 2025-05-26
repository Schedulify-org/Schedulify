#ifndef PRINT_SCHEDULE_H
#define PRINT_SCHEDULE_H

#include "model_interfaces.h"
#include "logger.h"

#include <QString>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>
#include <QPrintDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QDesktopServices>

// Function to print a schedule directly to a printer
bool printSelectedSchedule(const InformativeSchedule& schedule);

#endif // PRINT_SCHEDULE_H