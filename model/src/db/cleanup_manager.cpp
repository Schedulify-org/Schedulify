#include "cleanup_manager.h"

void CleanupManager::performCleanup() {
    Logger::get().logInfo("=== APPLICATION CLEANUP STARTED ===");

    // Check if Qt is still available for database operations
    if (!QCoreApplication::instance()) {
        Logger::get().logWarning("Qt Application already destroyed - skipping database cleanup");
        return;
    }

    try {
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (dbIntegration.isInitialized()) {
            auto& db = DatabaseManager::getInstance();
            if (db.isConnected()) {
                Logger::get().logInfo("Database connected - starting cleanup operations");

                // Clear all schedules but keep courses and files
                if (clearScheduleData(db)) {
                    Logger::get().logInfo("Schedule data cleared successfully");
                } else {
                    Logger::get().logWarning("Failed to clear some schedule data");
                }

                // Update metadata about cleanup
                try {
                    db.updateMetadata("last_cleanup",
                                      QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
                    Logger::get().logInfo("Cleanup metadata updated");
                } catch (const std::exception& e) {
                    Logger::get().logWarning("Failed to update cleanup metadata: " + std::string(e.what()));
                }
            } else {
                Logger::get().logInfo("Database was not connected - no cleanup needed");
            }

            // Use force cleanup to ensure proper shutdown
            try {
                Logger::get().logInfo("Performing force cleanup...");
                db.forceCleanup();
                Logger::get().logInfo("Force cleanup completed successfully");
            } catch (const std::exception& e) {
                Logger::get().logWarning("Exception during force cleanup: " + std::string(e.what()));
            }
        } else {
            Logger::get().logInfo("Database was not initialized - no cleanup needed");
        }
    } catch (const std::exception& e) {
        Logger::get().logError("Exception during database cleanup: " + std::string(e.what()));
    }

    Logger::get().logInfo("=== APPLICATION CLEANUP COMPLETED ===");
}

bool CleanupManager::clearScheduleData(DatabaseManager& db) {
    bool success = true;

    try {
        // Don't use transactions during shutdown to avoid potential issues
        // Clear schedules first (due to foreign key constraints)
        Logger::get().logInfo("Clearing schedules from database...");

        if (!db.schedules()->deleteAllSchedules()) {
            Logger::get().logWarning("Failed to delete all schedules");
            success = false;
        } else {
            Logger::get().logInfo("All schedules deleted");
        }

        // Clear schedule sets using the public interface
        Logger::get().logInfo("Clearing schedule sets from database...");

        // Get all schedule sets first, then delete them one by one
        auto scheduleSets = db.schedules()->getAllScheduleSets();
        int deletedSets = 0;

        for (const auto& scheduleSet : scheduleSets) {
            if (db.schedules()->deleteScheduleSet(scheduleSet.id)) {
                deletedSets++;
            } else {
                Logger::get().logWarning("Failed to delete schedule set ID: " + std::to_string(scheduleSet.id));
                success = false;
            }
        }

        Logger::get().logInfo("Deleted " + std::to_string(deletedSets) + " schedule sets");

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during schedule cleanup: " + std::string(e.what()));
        success = false;
    }

    return success;
}