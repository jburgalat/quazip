#ifndef QUAZIPTREEMODEL_HPP
#define QUAZIPTREEMODEL_HPP
#include "quazip/quazipfileinfo.h"
#include <QAbstractItemModel>

class TreeItem {
  public:
    explicit TreeItem(const QuaZipFileInfo64 &mInfos, TreeItem *parent = 0);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    TreeItem *appendChild(TreeItem *item);
    bool insertChildren(int position, QVector<TreeItem *> items);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool isDirectory() const;
    Qt::CheckState state() const;
    void setState(const Qt::CheckState &state);

    QuaZipFileInfo64 &infos();

    QStringList checkedPaths();

  private:
    QList<TreeItem *> mChildren;
    QuaZipFileInfo64 mInfos;
    TreeItem *mParentItem;
    Qt::CheckState mState;
};
class QuazipTreeModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    QuazipTreeModel(QObject *parent = 0);
    ~QuazipTreeModel();
    void setZipInfos(QList<QuaZipFileInfo64> infos);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    bool insertItems(int position, QVector<TreeItem *> items, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

    static QStringList mHeaders;

    void clear();
    QStringList checkedPaths();
    bool allChecked() const;
    bool allUnchecked() const;

  public slots:
    void checkAllItems();
    void uncheckAllItems();

  private:
    TreeItem *getItem(const QModelIndex &index) const;

  private:
    TreeItem *mRootItem;
    bool isDirectory(QString name, int &level);

    TreeItem *getParentItem(QString name, TreeItem *first);
    void setRecursiveState(const QModelIndex &index, Qt::CheckState);
    bool _checkAllStates(TreeItem *root, Qt::CheckState stateToCheck) const;
};

#endif // QUAZIPTREEMODEL_HPP
