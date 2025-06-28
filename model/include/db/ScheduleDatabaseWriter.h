#ifndef SCHEDULE_DATABASE_WRITER_H
#define SCHEDULE_DATABASE_WRITER_H

#include "model_interfaces.h"
#include "model_db_integration.h"
#include "logger.h"
#include <vector>
#include <string>
#include <memory>

class ScheduleDatabaseWriter {
public:
    static ScheduleDatabaseWriter& getInstance();

    // Initialize for a new schedule generation session
    bool initializeSession(const std::string& setName, const std::vector<int>& sourceFileIds);

    // Write a single schedule during generation
    bool writeSchedule(const InformativeSchedule& schedule);

    // Finalize the session (update counts, etc.)
    bool finalizeSession();

    // Get the current session's set ID
    int getCurrentSetId() const { return currentSetId; }

    // Get statistics about the current session
    struct SessionStats {
        int totalSchedulesWritten = 0;
        int successfulWrites = 0;
        int failedWrites = 0;
        bool sessionActive = false;
    };

    SessionStats getSessionStats() const { return sessionStats; }

    // Batch writing for better performance
    void setBatchSize(int size) { batchSize = size; }
    bool flushBatch(); // Manually flush current batch

private:
    ScheduleDatabaseWriter() = default;
    ~ScheduleDatabaseWriter();

    // Disable copy/move
    ScheduleDatabaseWriter(const ScheduleDatabaseWriter&) = delete;
    ScheduleDatabaseWriter& operator=(const ScheduleDatabaseWriter&) = delete;

    // Internal state
    bool sessionActive = false;
    int currentSetId = -1;
    std::string currentSetName;
    std::vector<int> currentSourceFileIds;
    SessionStats sessionStats;

    // Batch writing
    int batchSize = 100; // Write in batches of 100
    std::vector<InformativeSchedule> currentBatch;

    // Internal methods
    bool writeBatchToDatabase();
    void resetSession();
};

#endif // SCHEDULE_DATABASE_WRITER_H