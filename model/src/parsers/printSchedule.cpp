#include "parsers/printSchedule.h"
#include "logs/logger.h"
#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPageSize>
#include <QPageLayout>


void saveToPDF(const InformativeSchedule& schedule, const QString& filePath) {
    QPdfWriter pdfWriter(filePath);
    // Set A4 page size
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    // Set landscape orientation separately
    pdfWriter.setPageOrientation(QPageLayout::Landscape);
    // Reduce margins to maximize usable space
    pdfWriter.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
    pdfWriter.setTitle("Schedule Export");
    // Set resolution higher for better quality
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    QTextDocument document;

    // Set a larger base font size for better readability
    document.setDefaultFont(QFont("Arial", 12));

    // Set up the HTML content for the PDF
    QString html = "<html><body style='margin:0; padding:0;'>";
    html += "<h1 style='text-align:center; font-size:24pt; margin:5px 0 15px 0; color:#000000;'>Academic Schedule</h1>";

    // Set table styles for better readability - make full width, fixed layout, with larger text
    html += "<table border='1' cellspacing='0' cellpadding='8' width='100%' style='border-collapse:collapse; table-layout:fixed;'>";

    // Add table header with custom styling - darker header for better contrast
    html += "<tr bgcolor='#2c3e50' style='color:white;'>";
    html += "<th style='width:10%; font-size:14pt; padding:10px;'>Hours</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Sunday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Monday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Tuesday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Wednesday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Thursday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Friday</th>";
    html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Saturday</th>";
    html += "</tr>";

    // Define time slots
    const QStringList timeSlots = {
            "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
            "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
            "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
    };

    // Generate table rows
    for (const auto& timeSlot : timeSlots) {
        html += "<tr>";
        html += "<td bgcolor='#34495e' style='text-align:center; font-size:13pt; color:white; padding:8px;'><b>" + timeSlot + "</b></td>";

        // For each day of the week
        for (int day = 0; day < 7; day++) {
            html += "<td style='vertical-align:top; padding:6px;'>";

            // Extract hour range from time slot (e.g., "8:00-9:00" -> 8 to 9)
            int slotStart = timeSlot.split("-")[0].split(":")[0].toInt();
            int slotEnd = timeSlot.split("-")[1].split(":")[0].toInt();

            // Check if we have enough days in the schedule
            if (day < static_cast<int>(schedule.week.size())) {
                for (const auto& item : schedule.week[day].day_items) {
                    // Parse start and end times
                    int itemStart = std::stoi(item.start.substr(0, item.start.find(":")));
                    int itemEnd = std::stoi(item.end.substr(0, item.end.find(":")));

                    // Check if this item falls within current time slot
                    if ((itemStart <= slotStart && itemEnd > slotStart) ||
                        (itemStart >= slotStart && itemStart < slotEnd)) {
                        // Use a more vibrant color scheme for better readability
                        html += "<div style='background-color:#3498db; color:white; padding:6px; margin-bottom:6px; border-radius:4px; box-shadow: 1px 1px 3px rgba(0,0,0,0.2);'>";
                        html += "<strong style='font-size:13pt;'>" + QString::fromStdString(item.courseName) + "</strong><br/>";
                        html += "<span style='font-size:12pt;'>" + QString::fromStdString(item.raw_id) + " - " + QString::fromStdString(item.type) + "</span><br/>";
                        html += "<span style='font-size:12pt;'><b>" + QString::fromStdString(item.start) + " - " + QString::fromStdString(item.end) + "</b></span><br/>";
                        html += "<span style='font-size:12pt;'>Building: <b>" + QString::fromStdString(item.building) +
                                "</b>, Room: <b>" + QString::fromStdString(item.room) + "</b></span>";
                        html += "</div>";
                    }
                }
            }

            html += "</td>";
        }

        html += "</tr>";
    }

    html += "</table>";
    html += "<p style='text-align:right; font-style:italic; margin-top:10px; font-size:12pt;'>Generated on: " +
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
    html += "</body></html>";

    document.setHtml(html);
    // Use the page size from the PDF writer
    document.setPageSize(QSizeF(pdfWriter.width(), pdfWriter.height()));
    document.drawContents(&painter);

    painter.end();

    Logger::get().logInfo("Schedule successfully saved to PDF: " + filePath.toStdString());
}

void printSchedule(const InformativeSchedule& schedule) {
    QPrinter printer;

    // Setup printer with A4 size
    printer.setPageSize(QPageSize(QPageSize::A4));
    // Set landscape orientation separately
    printer.setPageOrientation(QPageLayout::Landscape);
    // Reduce margins to maximize usable space
    printer.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
    // Set higher resolution for better print quality
    printer.setResolution(300);

    // Create print dialog
    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QTextDocument document;

        // Set a larger base font size for better readability
        document.setDefaultFont(QFont("Arial", 12));

        // Set up HTML content for printing - similar to PDF export
        QString html = "<html><body style='margin:0; padding:0;'>";
        html += "<h1 style='text-align:center; font-size:24pt; margin:5px 0 15px 0; color:#000000;'>Academic Schedule</h1>";

        // Set table styles for better readability - make full width, fixed layout
        html += "<table border='1' cellspacing='0' cellpadding='8' width='100%' style='border-collapse:collapse; table-layout:fixed;'>";

        // Add table header with custom styling - darker header for better contrast
        html += "<tr bgcolor='#2c3e50' style='color:white;'>";
        html += "<th style='width:10%; font-size:14pt; padding:10px;'>Hours</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Sunday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Monday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Tuesday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Wednesday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Thursday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Friday</th>";
        html += "<th style='width:12.85%; font-size:14pt; padding:10px;'>Saturday</th>";
        html += "</tr>";

        // Define time slots
        const QStringList timeSlots = {
                "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
                "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
                "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
        };

        // Generate table rows
        for (const auto& timeSlot : timeSlots) {
            html += "<tr>";
            html += "<td bgcolor='#34495e' style='text-align:center; font-size:13pt; color:white; padding:8px;'><b>" + timeSlot + "</b></td>";

            // For each day of the week
            for (int day = 0; day < 7; day++) {
                html += "<td style='vertical-align:top; padding:6px;'>";

                // Extract hour range from time slot
                int slotStart = timeSlot.split("-")[0].split(":")[0].toInt();
                int slotEnd = timeSlot.split("-")[1].split(":")[0].toInt();

                // Check if we have enough days in the schedule
                if (day < static_cast<int>(schedule.week.size())) {
                    for (const auto& item : schedule.week[day].day_items) {
                        // Parse start and end times
                        int itemStart = std::stoi(item.start.substr(0, item.start.find(":")));
                        int itemEnd = std::stoi(item.end.substr(0, item.end.find(":")));

                        // Check if this item falls within current time slot
                        if ((itemStart <= slotStart && itemEnd > slotStart) ||
                            (itemStart >= slotStart && itemStart < slotEnd)) {
                            // Use a more vibrant color scheme for better readability
                            html += "<div style='background-color:#3498db; color:white; padding:6px; margin-bottom:6px; border-radius:4px; box-shadow: 1px 1px 3px rgba(0,0,0,0.2);'>";
                            html += "<strong style='font-size:13pt;'>" + QString::fromStdString(item.courseName) + "</strong><br/>";
                            html += "<span style='font-size:12pt;'>" + QString::fromStdString(item.raw_id) + " - " + QString::fromStdString(item.type) + "</span><br/>";
                            html += "<span style='font-size:12pt;'><b>" + QString::fromStdString(item.start) + " - " + QString::fromStdString(item.end) + "</b></span><br/>";
                            html += "<span style='font-size:12pt;'>Building: <b>" + QString::fromStdString(item.building) +
                                    "</b>, Room: <b>" + QString::fromStdString(item.room) + "</b></span>";
                            html += "</div>";
                        }
                    }
                }

                html += "</td>";
            }

            html += "</tr>";
        }

        html += "</table>";
        html += "<p style='text-align:right; font-style:italic; margin-top:10px; font-size:12pt;'>Generated on: " +
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
        html += "</body></html>";

        document.setHtml(html);
        // Set proper page size and use correct unit parameter
        document.setPageSize(printer.pageRect(QPrinter::Point).size());
        document.drawContents(&painter);

        Logger::get().logInfo("Schedule successfully sent to printer");
    }
}