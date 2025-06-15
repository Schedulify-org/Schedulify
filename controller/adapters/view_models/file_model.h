#ifndef FILE_HISTORY_MODEL_H
#define FILE_HISTORY_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "db_entities.h" // Changed from db_manager.h to db_entities.h

class FileHistoryModel : public QAbstractListModel {
Q_OBJECT

public:
    enum FileRoles {
        IdRole = Qt::UserRole + 1,
        FileNameRole,
        FileTypeRole,
        UploadTimeRole,
        UpdatedAtRole,
        FormattedDateRole
    };

    explicit FileHistoryModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Custom methods
    void populateFiles(const vector<FileEntity>& files);
    void clearFiles();
    int getFileId(int index) const;
    void forceRefresh();

    // Debug methods
    void logModelState() const;
    bool validateIndex(int index) const;

private:
    vector<FileEntity> m_files;
    QString formatDateTime(const QDateTime& dateTime) const;
};

#endif // FILE_HISTORY_MODEL_H