#include "printSchedule.h"

bool printSelectedSchedule(const InformativeSchedule& schedule) {
    QPrinter printer;
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(2, 2, 2, 2), QPageLayout::Millimeter);
    printer.setResolution(300);

    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QTextDocument document;
        document.setDefaultFont(QFont("Arial", 8));

        QString html = "<html><body style='margin:0; padding:0;'>";
        html += "<h1 style='text-align:center; font-size:12pt; margin:3px 0 15px 0; color:#000000;'>Academic Schedule</h1>";

        html += "<table border='1' cellspacing='0' cellpadding='4' width='100%' style='border-collapse:collapse; table-layout:fixed;'>";

        html += "<tr bgcolor='#2c3e50' style='color:white;'>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Hours</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Sunday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Monday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Tuesday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Wednesday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Thursday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Friday</th>";
        html += "<th style='width:10%; font-size:12pt; padding:7px;'>Saturday</th>";
        html += "</tr>";

        const QStringList timeSlots = {
            "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
            "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
            "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
        };

        for (const auto& timeSlot : timeSlots) {
            html += "<tr>";
            html += "<td bgcolor='#34495e' style='text-align:center; font-size:10pt; color:white; padding:3px;'><b>" + timeSlot + "</b></td>";

            for (int day = 0; day < 7; day++) {
                html += "<td style='vertical-align:top; padding:2px;'>";

                int slotStart = timeSlot.split("-")[0].split(":")[0].toInt();
                int slotEnd = timeSlot.split("-")[1].split(":")[0].toInt();

                if (day < static_cast<int>(schedule.week.size())) {
                    for (const auto& item : schedule.week[day].day_items) {
                        int itemStart = std::stoi(item.start.substr(0, item.start.find(":")));
                        int itemEnd = std::stoi(item.end.substr(0, item.end.find(":")));

                        if ((itemStart <= slotStart && itemEnd > slotStart) ||
                            (itemStart >= slotStart && itemStart < slotEnd)) {
                            html += "<div style='background-color:#1f2937; color:#ffffff; padding:2px; margin-bottom:2px; border-radius:2px;'>";
                            html += "<strong style='font-size:9pt;'>" + QString::fromStdString(item.courseName) + "</strong><br/>";
                            html += "<span style='font-size:7pt;'>" + QString::fromStdString(item.raw_id) + " - " + QString::fromStdString(item.type) + "</span><br/>";
                            html += "<span style='font-size:7pt;'><b>" + QString::fromStdString(item.start) + " - " + QString::fromStdString(item.end) + "</b></span><br/>";
                            html += "<span style='font-size:7pt;'>Building: <b>" + QString::fromStdString(item.building) +
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
        html += "<p style='text-align:right; font-style:italic; margin-top:8px; font-size:8pt;'>Generated on: " +
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
        html += "</body></html>";

        document.setHtml(html);
        document.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
        document.drawContents(&painter);
        painter.end();

        Logger::get().logInfo("Schedule successfully sent to printer.");
        return true;
    } else {
        Logger::get().logInfo("Aborting printing process.");
        return false;
    }
}
