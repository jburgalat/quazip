#include "quaziptreemodel.hpp"
#include <QFileInfo>
#include <QMutableListIterator>
#include <QtWidgets>

/// @brief Constructor.
TreeItem::TreeItem(const QuaZipFileInfo64 &infos, TreeItem *parent) {
    mParentItem = parent;
    mInfos = infos;
    mState = Qt::Checked;
}
/// @brief Destruuctor.
TreeItem::~TreeItem() { qDeleteAll(mChildren); }

/// @brief Get the child at given index.
TreeItem *TreeItem::child(int number) { return mChildren.value(number); }

/// @brief Get the total count of children.
int TreeItem::childCount() const { return mChildren.count(); }

/// @brief Get this child index in parent children.
int TreeItem::childNumber() const {
    if (mParentItem)
        return mParentItem->mChildren.indexOf(const_cast<TreeItem *>(this));
    return 0;
}

QStringList TreeItem::checkedPaths() {
    QStringList out;
    if (mState == Qt::Checked)
        out << mInfos.name;
    for (TreeItem *item : mChildren)
        out << item->checkedPaths();
    return out;
}

//// @brief Check whether or not the item is a directory.
bool TreeItem::isDirectory() const { return mInfos.name.endsWith("/"); }

/// @brief Get the data at given column.
QVariant TreeItem::data(int column) const {
    bool isDir = isDirectory();
    QString name = mInfos.name;
    if (isDir)
        name = name.left(name.length() - 1);
    name = name.mid(name.lastIndexOf("/") + 1);

    switch (column) {
    case 0: // name
        return name;
    case 1: // real size
        return isDir ? QVariant() : QVariant(QString("%1 bytes").arg(mInfos.uncompressedSize));
    case 2: //
        return isDir ? QVariant()
                     : QVariant(QString("%1 %").arg(
                           100.0 * (1.0 - (double)mInfos.compressedSize / mInfos.uncompressedSize), 6, 'f', 2));
    default:
        return QVariant();
    }
}

/// @brief Append new children.
TreeItem *TreeItem::appendChild(TreeItem *item) {
    if (item->mParentItem != this)
        item->mParentItem = this;
    mChildren.append(item);
    return item;
}

/// @brief Insert childrens at given position
bool TreeItem::insertChildren(int position, QVector<TreeItem *> items) {
    if (position < 0 || position > mChildren.size())
        return false;
    int count = items.size();
    for (int row = 0; row < count; ++row) {
        TreeItem *item = items[row];
        item->mParentItem = this;
        mChildren.insert(position, item);
    }
    return true;
}

/// @brief Get the parent of this item.
TreeItem *TreeItem::parent() { return mParentItem; }

/// @brief Remove children.
bool TreeItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > mChildren.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete mChildren.takeAt(position);

    return true;
}

/// @brief Ge tthe checked state.
Qt::CheckState TreeItem::state() const { return mState; }

/// @brief Set the checkstate value.
void TreeItem::setState(const Qt::CheckState &state) {
    mState = state;
    if (isDirectory()) {
        for (TreeItem *item : mChildren)
            item->setState(state);
    }
}

/// @brief Get the infos of the file.
QuaZipFileInfo64 &TreeItem::infos() { return mInfos; }

/// @brief List of model headers.
QStringList QuazipTreeModel::mHeaders = QStringList() << "File Name"
                                                      << "Actual size"
                                                      << "Compression Ratio";

/// @brief Constructor.
QuazipTreeModel::QuazipTreeModel(QObject *parent) : QAbstractItemModel(parent) {
    mRootItem = new TreeItem(QuaZipFileInfo64(), Q_NULLPTR);
    mRootItem->infos().name = "";
}

/// @brief Destructor.
QuazipTreeModel::~QuazipTreeModel() { delete mRootItem; }

/// @brief Check if the name is related to a directory.
bool QuazipTreeModel::isDirectory(QString name, int &level) {
    level = name.count("/");
    return name.endsWith("/");
}

/// @brief Clears the model.
void QuazipTreeModel::clear() {
    beginResetModel();
    if (mRootItem)
        delete mRootItem;
    mRootItem = new TreeItem(QuaZipFileInfo64(), Q_NULLPTR);
    endResetModel();
}

/// @brief Check all items of the model.
void QuazipTreeModel::checkAllItems() {
    for (int i = 0; i < mRootItem->childCount(); i++)
        setData(index(i, 0), static_cast<int>(Qt::Checked), Qt::CheckStateRole);
}

/// @brief Uncheck all items of the model.
void QuazipTreeModel::uncheckAllItems() {
    for (int i = 0; i < mRootItem->childCount(); i++)
        setData(index(i, 0), static_cast<int>(Qt::Unchecked), Qt::CheckStateRole);
}

/// @brief Initialize the model with list of infos
void QuazipTreeModel::setZipInfos(QList<QuaZipFileInfo64> infos) {
    beginResetModel();
    if (mRootItem)
        delete mRootItem;
    mRootItem = new TreeItem(QuaZipFileInfo64(), Q_NULLPTR);
    TreeItem *lastParent = mRootItem;
    QMutableListIterator<QuaZipFileInfo64> it(infos);
    while (it.hasNext()) {
        QuaZipFileInfo64 info = it.next();
        lastParent = getParentItem(info.name, mRootItem);
        TreeItem *current = new TreeItem(info, lastParent);
        lastParent->appendChild(current);
        it.remove();
    }
    endResetModel();
}

/// @brief Get the parent item from name.
TreeItem *QuazipTreeModel::getParentItem(QString name, TreeItem *first) {
    int lvl;
    bool isdir = isDirectory(name, lvl);
    if (isdir && lvl == 1)
        return mRootItem;
    QString cname = name;
    if (isdir)
        cname = name.left(name.length() - 1);
    cname = cname.left(cname.lastIndexOf("/") + 1);

    for (int i = 0; i < first->childCount(); i++) {
        QString curName = first->child(i)->infos().name;
        if (curName.localeAwareCompare(cname) == 0)
            return first->child(i);
        else if (name.startsWith(curName)) {
            return getParentItem(name, first->child(i));
        }
    }
    return mRootItem;
}

/// @brief Get the list of all checked items.
QStringList QuazipTreeModel::checkedPaths() {
    if (allChecked() || allUnchecked())
        return QStringList();
    return mRootItem->checkedPaths();
}

/// @brief Check if all items are checked in the model.
bool QuazipTreeModel::allChecked() const { return _checkAllStates(mRootItem, Qt::Checked); }

/// @brief Check if all items are unchecked in the model.
bool QuazipTreeModel::allUnchecked() const { return _checkAllStates(mRootItem, Qt::Unchecked); }

/// @brief Number of column in the model.
int QuazipTreeModel::columnCount(const QModelIndex & /* parent */) const { return mHeaders.size(); }

/// @brief Get the data at given index.
QVariant QuazipTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    TreeItem *item = getItem(index);
    QStyle *style = QApplication::style();
    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        return item->isDirectory() ? style->standardIcon(QStyle::SP_DirIcon) : style->standardIcon(QStyle::SP_FileIcon);
    } else if (role == Qt::CheckStateRole && index.column() == 0)
        return item->state();
    else
        return QVariant();
}

/// @brief Get the flags of the model for the given index.
Qt::ItemFlags QuazipTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsUserCheckable;
    return f;
}

/// @brief Get item at given index.
TreeItem *QuazipTreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        if (item)
            return item;
    }
    return mRootItem;
}
//! [4]

QVariant QuazipTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mHeaders[section];
    return QVariant();
}

/// @brief Create an index for the given row,column and parent index.
QModelIndex QuazipTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

/// @brief Insert new rows in the model.
bool QuazipTreeModel::insertItems(int position, QVector<TreeItem *> items, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success;
    int rows = items.size();
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, items);
    endInsertRows();
    return success;
}

/// @brief Get the parent index of the given index.
QModelIndex QuazipTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();
    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();
    if (parentItem == mRootItem)
        return QModelIndex();
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

/// @brief Remove rows from the model.
bool QuazipTreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

/// Get the number of row under the given index.
int QuazipTreeModel::rowCount(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

/// @brief Set data.
bool QuazipTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::CheckStateRole && index.column() != 0)
        return false;

    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    // setRecursiveState(index, state);
    // return true;

    TreeItem *item = getItem(index);
    item->setState(state);
    emit dataChanged(index, index);
    if (item->isDirectory()) {
        emit dataChanged(index.child(0, 0), index.child(rowCount(index), 0));
    }
    return true;
}

/// @brief Set new check state recursively in the model.
void QuazipTreeModel::setRecursiveState(const QModelIndex &index, Qt::CheckState state) {
    if (!index.isValid())
        return;
    TreeItem *item = getItem(index);
    item->setState(state);
    emit dataChanged(index, index);
    if (item->isDirectory()) {
        for (int i = 0; i < rowCount(index); i++) {
            setRecursiveState(index.child(i, 0), state);
        }
    }
}

/// @brief Check for states recursively.
bool QuazipTreeModel::_checkAllStates(TreeItem *root, Qt::CheckState stateToCheck) const {
    bool ret = (root != mRootItem) ? (root->state() == stateToCheck) : true;
    if (!ret)
        return false;
    else
        for (int i = 0; i < root->childCount(); i++)
            ret = ret && _checkAllStates(root->child(i), stateToCheck);
    return ret;
}
