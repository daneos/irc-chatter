
// This file is part of IRC Chatter, the first IRC Client for MeeGo.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation
// and appearing in the file LICENSE.LGPL included in the packaging
// of this file.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// Copyright (c) 2012, Timur Kristóf <venemo@fedoraproject.org>

#ifndef QOBJECTLISTMODEL_H
#define QOBJECTLISTMODEL_H

#include <QtCore/QAbstractListModel>

class QObjectListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

    QList<QObject*> *_list;
    QHash<int, QByteArray> _roles;

public:
    explicit QObjectListModel(QObject *parent = 0, QList<QObject*> *list = new QList<QObject*>());
    virtual ~QObjectListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int itemCount() const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE void reset();

    void addItem(QObject *item);
    void removeItem(QObject *item);
    void removeItem(int index);
    Q_INVOKABLE QObject* getItem(int index);
    int indexOf(QObject *obj) const;

    template<typename T>
    QList<T*> *getList();
    QList<QObject*> *getList();

    template<typename T>
    void setList(QList<T*> *list);
    void setList(QList<QObject*> *list);
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const { return _roles; }
#endif

private slots:
    void removeDestroyedItem();

signals:
    void itemAdded(QObject *item);
    void itemCountChanged();
};

template<typename T>
QList<T*> *QObjectListModel::getList()
{
    return reinterpret_cast<QList<T *> *>(_list);
}

template<typename T>
void QObjectListModel::setList(QList<T*> *list)
{
    setList(reinterpret_cast<QList<QObject *> *>(list));
}

#endif // QOBJECTLISTMODEL_H
