#include "parsers/printSchedule.h"
#include "logs/logger.h"
#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QPrintDialog>
#include <QPageSize>
#include <QPageLayout>

void saveToPDF(const InformativeSchedule& schedule, const QString& filePath) {
    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);
    pdfWriter.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
    pdfWriter.setTitle("Schedule Export");
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    QTextDocument document;
    document.setDefaultFont(QFont("Arial", 14));

    QString html = "<html><body style='margin:0; padding:0;'>";
    html += "<h1 style='text-align:center; font-size:30pt; margin:5px 0 20px 0; color:#1f2937;'>Academic Schedule</h1>";
    html += "<table border='1' cellspacing='0' cellpadding='10' width='100%' style='border-collapse:collapse; table-layout:fixed;'>";
    html += "<tr bgcolor='#1f2937' style='color:white;'>";
    html += "<th style='width:10%; font-size:16pt; padding:12px;'>Hours</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Sunday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Monday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Tuesday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Wednesday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Thursday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Friday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Saturday</th>";
    html += "</tr>";

    const QStringList timeSlots = {
        "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
        "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
        "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
    };

    for (const auto& timeSlot : timeSlots) {
        html += "<tr>";
        html += "<td bgcolor='#35455c' style='text-align:center; font-size:15pt; color:white; padding:10px;'><b>" + timeSlot + "</b></td>";
        for (int day = 0; day < 7; day++) {
            html += "<td style='vertical-align:top; padding:8px;'>";
            int slotStart = timeSlot.split("-")[0].split(":")[0].toInt();
            int slotEnd = timeSlot.split("-")[1].split(":")[0].toInt();
            if (day < static_cast<int>(schedule.week.size())) {
                for (const auto& item : schedule.week[day].day_items) {
                    int itemStart = std::stoi(item.start.substr(0, item.start.find(":")));
                    int itemEnd = std::stoi(item.end.substr(0, item.end.find(":")));
                    if ((itemStart <= slotStart && itemEnd > slotStart) ||
                        (itemStart >= slotStart && itemStart < slotEnd)) {
                        html += "<div style='background-color:#1f2937; color:white; padding:8px; margin-bottom:8px; border-radius:4px; box-shadow: 2px 2px 4px rgba(0,0,0,0.3);'>";
                        html += "<strong style='font-size:15pt;'>" + QString::fromStdString(item.courseName) + "</strong><br/>";
                        html += "<span style='font-size:14pt;'>" + QString::fromStdString(item.raw_id) + " - " + QString::fromStdString(item.type) + "</span><br/>";
                        html += "<span style='font-size:14pt;'><b>" + QString::fromStdString(item.start) + " - " + QString::fromStdString(item.end) + "</b></span><br/>";
                        html += "<span style='font-size:14pt;'>Building: <b>" + QString::fromStdString(item.building) +
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
    html += "<p style='text-align:right; font-style:italic; margin-top:15px; font-size:14pt;'>Generated on: " +
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
    html += "</body></html>";

    document.setHtml(html);
    document.setPageSize(QSizeF(pdfWriter.width(), pdfWriter.height()));
    document.drawContents(&painter);

    painter.end();
    Logger::get().logInfo("Schedule successfully saved to PDF: " + filePath.toStdString());
}

void printSchedule(const InformativeSchedule& schedule) {
    // Create temporary PDF
    QString tempPdfPath = QDir::temp().filePath("temp_schedule_print.pdf");
    saveToPDF(schedule, tempPdfPath);

    // Check if PDF was created
    QFile tempFile(tempPdfPath);
    if (!tempFile.exists()) {
        Logger::get().logError("Failed to generate PDF for printing.");
        return;
    }

    // Set up printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
    printer.setFullPage(true);
    printer.setColorMode(QPrinter::Color);

    // Show print dialog
    QPrintDialog printDialog(&printer);
    if (printDialog.exec() != QDialog::Accepted) {
        Logger::get().logInfo("Print operation cancelled by user.");
        tempFile.remove();
        return;
    }

    // Generate HTML content (same as in saveToPDF)
    QTextDocument document;
    document.setDefaultFont(QFont("Arial", 14));
    QString html = "<html><body style='margin:0; padding:0;'>";
    html += "<h1 style='text-align:center; font-size:30pt; margin:5px 0 20px 0; color:#1f2937;'>Academic Schedule</h1>";
    html += "<table border='1' cellspacing='0' cellpadding='10' width='100%' style='border-collapse:collapse; table-layout:fixed;'>";
    html += "<tr bgcolor='#1f2937' style='color:white;'>";
    html += "<th style='width:10%; font-size:16pt; padding:12px;'>Hours</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Sunday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Monday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Tuesday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Wednesday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Thursday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Friday</th>";
    html += "<th style='width:12.85%; font-size:16pt; padding:12px;'>Saturday</th>";
    html += "</tr>";

    const QStringList timeSlots = {
        "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
        "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
        "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
    };

    for (const auto& timeSlot : timeSlots) {
        html += "<tr>";
        html += "<td bgcolor='#35455c' style='text-align:center; font-size:15pt; color:white; padding:10px;'><b>" + timeSlot + "</b></td>";
        for (int day = 0; day < 7; day++) {
            html += "<td style='vertical-align:top; padding:8px;'>";
            int slotStart = timeSlot.split("-")[0].split(":")[0].toInt();
            int slotEnd = timeSlot.split("-")[1].split(":")[0].toInt();
            if (day < static_cast<int>(schedule.week.size())) {
                for (const auto& item : schedule.week[day].day_items) {
                    int itemStart = std::stoi(item.start.substr(0, item.start.find(":")));
                    int itemEnd = std::stoi(item.end.substr(0, item.end.find(":")));
                    if ((itemStart <= slotStart && itemEnd > slotStart) ||
                        (itemStart >= slotStart && itemStart < slotEnd)) {
                        html += "<div style='background-color:#1f2937; color:white; padding:8px; margin-bottom:8px; border-radius:4px; box-shadow: 2px 2px 4px rgba(0,0,0,0.3);'>";
                        html += "<strong style='font-size:15pt;'>" + QString::fromStdString(item.courseName) + "</strong><br/>";
                        html += "<span style='font-size:14pt;'>" + QString::fromStdString(item.raw_id) + " - " + QString::fromStdString(item.type) + "</span><br/>";
                        html += "<span style='font-size:14pt;'><b>" + QString::fromStdString(item.start) + " - " + QString::fromStdString(item.end) + "</b></span><br/>";
                        html += "<span style='font-size:14pt;'>Building: <b>" + QString::fromStdString(item.building) +
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
    html += "<p style='text-align:right; font-style:italic; margin-top:15px; font-size:14pt;'>Generated on: " +
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
    html += "</body></html>";

    // Debug: Log HTML content to verify it's not empty
    if (html.isEmpty()) {
        Logger::get().logError("Generated HTML for printing is empty.");
        tempFile.remove();
        return;
    }
    Logger::get().logInfo("HTML content generated for printing. Length: " + std::to_string(html.length()));

    // Set up document for printing
    document.setHtml(html);
    document.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());

    // Print the document
    QPainter painter;
    if (!painter.begin(&printer)) {
        Logger::get().logError("Failed to initialize printer.");
        tempFile.remove();
        return;
    }

    document.drawContents(&painter);
    painter.end();

    // Clean up temporary file
    tempFile.remove();
    Logger::get().logInfo("Schedule printed successfully.");
}