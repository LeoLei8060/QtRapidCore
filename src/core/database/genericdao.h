#ifndef GENERICDAO_H
#define GENERICDAO_H

#include <QDebug>
#include <QMap>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QSharedPointer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

namespace Core {
namespace Database {

// 翻页结果结构体
template<typename T>
struct PageResult
{
    QVector<T> data;        // 当前页数据
    int        totalCount;  // 总记录数
    int        currentPage; // 当前页码（从1开始）
    int        pageSize;    // 每页大小
    int        totalPages;  // 总页数
    bool       hasNext;     // 是否有下一页
    bool       hasPrev;     // 是否有上一页

    PageResult()
        : totalCount(0)
        , currentPage(1)
        , pageSize(10)
        , totalPages(0)
        , hasNext(false)
        , hasPrev(false)
    {}
};

// 数据库字段特性
struct FieldAttribute
{
    bool isPrimaryKey = false;
    bool autoIncrement = false;
    bool notNull = false;
    bool unique = false;
};

// 前向声明
template<typename T>
class GenericDao;

// 查询构建器类
template<typename T>
class QueryBuilder
{
public:
    QueryBuilder(GenericDao<T> *dao);

    // 条件查询
    QueryBuilder &where(const QString &field, const QVariant &value);
    QueryBuilder &where(const QString &field, const QString &op, const QVariant &value);
    QueryBuilder &whereAnd(const QString &field, const QVariant &value);
    QueryBuilder &whereAnd(const QString &field, const QString &op, const QVariant &value);
    QueryBuilder &whereOr(const QString &field, const QVariant &value);
    QueryBuilder &whereOr(const QString &field, const QString &op, const QVariant &value);
    QueryBuilder &whereLike(const QString &field, const QString &pattern);
    QueryBuilder &whereBetween(const QString &field, const QVariant &min, const QVariant &max);
    QueryBuilder &whereIn(const QString &field, const QVariantList &values);
    QueryBuilder &whereNull(const QString &field);
    QueryBuilder &whereNotNull(const QString &field);

    // 排序
    QueryBuilder &orderBy(const QString &field, const QString &direction = "ASC");
    QueryBuilder &orderByDesc(const QString &field);

    // 分页
    QueryBuilder &limit(int count);
    QueryBuilder &offset(int offset);
    QueryBuilder &limit(int count, int offset);

    // 分组
    QueryBuilder &groupBy(const QString &field);
    QueryBuilder &having(const QString &condition);

    // 执行查询
    QVector<T> get();
    T          first();
    int        count();

    // 翻页查询
    QSharedPointer<PageResult<T>> paginate(int page, int pageSize = 10);

    // 获取总数（不受 limit/offset 影响）
    int totalCount();

    // 重置构建器
    QueryBuilder &reset();

private:
    GenericDao<T> *m_dao;
    QStringList    m_whereConditions;
    QVariantList   m_bindValues;
    QStringList    m_orderBy;
    QString        m_groupBy;
    QString        m_having;
    int            m_limit;
    int            m_offset;

    QString buildSQL();
    void    addCondition(const QString &condition, const QVariant &value = QVariant());
    void    addCondition(const QString &condition, const QVariantList &values);
    // 构建计数SQL（去除 ORDER BY, LIMIT, OFFSET）
    QString buildCountSQL();
};

// 通用DAO类
template<typename T>
class GenericDao
{
public:
    GenericDao(const QString &dbPath, const QString &tableName);
    ~GenericDao();

    // 数据库操作
    bool       createTable();
    bool       insert(T &entity);
    bool       update(const T &entity);
    bool       remove(const T &entity);
    QVector<T> selectAll();
    T          selectById(QVariant id);

    // 查询构建器
    QueryBuilder<T> query();

    // 直接条件查询
    QVector<T> selectWhere(const QString &whereClause, const QVariantList &bindValues = QVariantList());

    // 事务支持
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // 简单的分页查询（无条件）
    QSharedPointer<PageResult<T>> selectPage(int page, int pageSize = 10);

    // 带条件的分页查询
    QSharedPointer<PageResult<T>> selectPageWhere(const QString      &whereClause,
                                                  const QVariantList &bindValues = QVariantList(),
                                                  int                 page = 1,
                                                  int                 pageSize = 10);

    // 错误处理
    QString lastError() const;

    // 友元类
    friend class QueryBuilder<T>;

private:
    void       initFieldMap();
    QString    buildCreateTableSQL();
    QString    buildInsertSQL(T &entity);
    QString    buildUpdateSQL(const T &entity);
    QString    buildDeleteSQL();
    QString    buildSelectSQL();
    QVector<T> executeSelectQuery(const QString &sql, const QVariantList &bindValues = QVariantList());

    QSqlDatabase                  m_db;
    QString                       m_tableName;
    QMap<QString, FieldAttribute> m_fieldAttributes;
    QString                       m_primaryKey;
    QString                       m_lastError;
};

// QueryBuilder 实现
template<typename T>
QueryBuilder<T>::QueryBuilder(GenericDao<T> *dao)
    : m_dao(dao)
    , m_limit(-1)
    , m_offset(-1)
{}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::where(const QString &field, const QVariant &value)
{
    return where(field, "=", value);
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::where(const QString &field, const QString &op, const QVariant &value)
{
    QString condition = QString("%1 %2 ?").arg(field).arg(op);
    addCondition(condition, value);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereAnd(const QString &field, const QVariant &value)
{
    return whereAnd(field, "=", value);
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereAnd(const QString &field, const QString &op, const QVariant &value)
{
    QString condition = QString("AND %1 %2 ?").arg(field).arg(op);
    addCondition(condition, value);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereOr(const QString &field, const QVariant &value)
{
    return whereOr(field, "=", value);
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereOr(const QString &field, const QString &op, const QVariant &value)
{
    QString condition = QString("OR %1 %2 ?").arg(field).arg(op);
    addCondition(condition, value);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereLike(const QString &field, const QString &pattern)
{
    QString condition = QString("%1 LIKE ?").arg(field);
    addCondition(condition, pattern);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereBetween(const QString &field, const QVariant &min, const QVariant &max)
{
    QString condition = QString("%1 BETWEEN ? AND ?").arg(field);
    addCondition(condition, QVariantList() << min << max);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereIn(const QString &field, const QVariantList &values)
{
    QStringList placeholders;
    for (int i = 0; i < values.size(); ++i) {
        placeholders << "?";
    }
    QString condition = QString("%1 IN (%2)").arg(field).arg(placeholders.join(", "));
    addCondition(condition, values);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereNull(const QString &field)
{
    QString condition = QString("%1 IS NULL").arg(field);
    addCondition(condition);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::whereNotNull(const QString &field)
{
    QString condition = QString("%1 IS NOT NULL").arg(field);
    addCondition(condition);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::orderBy(const QString &field, const QString &direction)
{
    m_orderBy << QString("%1 %2").arg(field).arg(direction);
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::orderByDesc(const QString &field)
{
    return orderBy(field, "DESC");
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::limit(int count)
{
    m_limit = count;
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::offset(int offset)
{
    m_offset = offset;
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::limit(int count, int offset)
{
    m_limit = count;
    m_offset = offset;
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::groupBy(const QString &field)
{
    m_groupBy = field;
    return *this;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::having(const QString &condition)
{
    m_having = condition;
    return *this;
}

template<typename T>
QVector<T> QueryBuilder<T>::get()
{
    QString sql = buildSQL();
    return m_dao->executeSelectQuery(sql, m_bindValues);
}

template<typename T>
T QueryBuilder<T>::first()
{
    limit(1);
    QVector<T> results = get();
    return results.isEmpty() ? T() : results.first();
}

template<typename T>
int QueryBuilder<T>::count()
{
    QString sql = QString("SELECT COUNT(*) FROM %1").arg(m_dao->m_tableName);

    if (!m_whereConditions.isEmpty()) {
        sql += " WHERE " + m_whereConditions.join(" ");
    }

    if (!m_groupBy.isEmpty()) {
        sql += " GROUP BY " + m_groupBy;
    }

    if (!m_having.isEmpty()) {
        sql += " HAVING " + m_having;
    }

    QSqlQuery query(m_dao->m_db);
    query.prepare(sql);

    for (const auto &value : m_bindValues) {
        query.addBindValue(value);
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// QueryBuilder 翻页方法实现
template<typename T>
QSharedPointer<PageResult<T>> QueryBuilder<T>::paginate(int page, int pageSize)
{
    QSharedPointer<PageResult<T>> result = QSharedPointer<PageResult<T>>::create();

    if (page < 1)
        page = 1;
    if (pageSize < 1)
        pageSize = 10;

    // 先获取总数
    int total = totalCount();

    // 计算翻页信息
    result->totalCount = total;
    result->currentPage = page;
    result->pageSize = pageSize;
    result->totalPages = (total + pageSize - 1) / pageSize; // 向上取整
    result->hasNext = page < result->totalPages;
    result->hasPrev = page > 1;

    // 如果没有数据，直接返回
    if (total == 0) {
        return result;
    }

    // 设置分页参数并查询数据
    int offset = (page - 1) * pageSize;
    limit(pageSize, offset);
    result->data = get();

    return result;
}

template<typename T>
int QueryBuilder<T>::totalCount()
{
    QString sql = buildCountSQL();

    QSqlQuery query(m_dao->m_db);
    query.prepare(sql);

    for (const auto &value : m_bindValues) {
        query.addBindValue(value);
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

template<typename T>
QString QueryBuilder<T>::buildCountSQL()
{
    QString sql = QString("SELECT COUNT(*) FROM %1").arg(m_dao->m_tableName);

    if (!m_whereConditions.isEmpty()) {
        sql += " WHERE " + m_whereConditions.join(" ");
    }

    if (!m_groupBy.isEmpty()) {
        sql += " GROUP BY " + m_groupBy;
        // 如果有 GROUP BY，需要用子查询包装
        sql = QString("SELECT COUNT(*) FROM (%1) AS subquery").arg(sql);
    }

    if (!m_having.isEmpty() && !m_groupBy.isEmpty()) {
        // HAVING 只在有 GROUP BY 时有效，需要在子查询中处理
        QString innerSql = QString("SELECT COUNT(*) FROM %1").arg(m_dao->m_tableName);
        if (!m_whereConditions.isEmpty()) {
            innerSql += " WHERE " + m_whereConditions.join(" ");
        }
        innerSql += " GROUP BY " + m_groupBy;
        innerSql += " HAVING " + m_having;
        sql = QString("SELECT COUNT(*) FROM (%1) AS subquery").arg(innerSql);
    }

    return sql;
}

template<typename T>
QueryBuilder<T> &QueryBuilder<T>::reset()
{
    m_whereConditions.clear();
    m_bindValues.clear();
    m_orderBy.clear();
    m_groupBy.clear();
    m_having.clear();
    m_limit = -1;
    m_offset = -1;
    return *this;
}

template<typename T>
QString QueryBuilder<T>::buildSQL()
{
    QString sql = QString("SELECT * FROM %1").arg(m_dao->m_tableName);

    if (!m_whereConditions.isEmpty()) {
        sql += " WHERE " + m_whereConditions.join(" ");
    }

    if (!m_groupBy.isEmpty()) {
        sql += " GROUP BY " + m_groupBy;
    }

    if (!m_having.isEmpty()) {
        sql += " HAVING " + m_having;
    }

    if (!m_orderBy.isEmpty()) {
        sql += " ORDER BY " + m_orderBy.join(", ");
    }

    if (m_limit > 0) {
        sql += QString(" LIMIT %1").arg(m_limit);
    }

    if (m_offset > 0) {
        sql += QString(" OFFSET %1").arg(m_offset);
    }

    return sql;
}

template<typename T>
void QueryBuilder<T>::addCondition(const QString &condition, const QVariant &value)
{
    m_whereConditions << condition;
    if (value.isValid()) {
        m_bindValues << value;
    }
}

template<typename T>
void QueryBuilder<T>::addCondition(const QString &condition, const QVariantList &values)
{
    m_whereConditions << condition;
    m_bindValues << values;
}

// GenericDao 实现部分
template<typename T>
GenericDao<T>::GenericDao(const QString &dbPath, const QString &tableName)
    : m_tableName(tableName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", QString("%1_%2").arg(tableName).arg(reinterpret_cast<quintptr>(this)));
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = "Failed to open database: " + m_db.lastError().text();
        qWarning() << m_lastError;
        return;
    }

    initFieldMap();
}

template<typename T>
GenericDao<T>::~GenericDao()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(m_db.connectionName());
}

template<typename T>
QueryBuilder<T> GenericDao<T>::query()
{
    return QueryBuilder<T>(this);
}

template<typename T>
QVector<T> GenericDao<T>::selectWhere(const QString &whereClause, const QVariantList &bindValues)
{
    QString sql = QString("SELECT * FROM %1 WHERE %2").arg(m_tableName).arg(whereClause);
    return executeSelectQuery(sql, bindValues);
}

template<typename T>
QVector<T> GenericDao<T>::executeSelectQuery(const QString &sql, const QVariantList &bindValues)
{
    QVector<T> results;
    QSqlQuery  query(m_db);

    query.prepare(sql);
    for (const auto &value : bindValues) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        m_lastError = "Select failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return results;
    }

    const QMetaObject *metaObj = &T::staticMetaObject;

    while (query.next()) {
        T entity;
        for (int i = 0; i < metaObj->propertyCount(); ++i) {
            QMetaProperty prop = metaObj->property(i);
            QString       propName = prop.name();

            if (propName == "objectName")
                continue;

            QVariant value = query.value(propName);
            if (value.isValid()) {
                prop.writeOnGadget(&entity, value);
            }
        }
        results.append(entity);
    }

    return results;
}

template<typename T>
void GenericDao<T>::initFieldMap()
{
    const QMetaObject *metaObj = &T::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        // 跳过QObject的属性
        if (propName == "objectName")
            continue;

        // 设置字段属性
        FieldAttribute attr;
        attr.isPrimaryKey = propName == "id";
        // attr.autoIncrement = propName == "id";
        m_fieldAttributes[propName] = attr;

        if (attr.isPrimaryKey) {
            m_primaryKey = propName;
        }
    }

    if (m_primaryKey.isEmpty()) {
        qWarning() << "No primary key defined for table" << m_tableName;
    }
}

template<typename T>
QString GenericDao<T>::buildCreateTableSQL()
{
    QStringList        columns;
    const QMetaObject *metaObj = &T::staticMetaObject;

    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        // 跳过QObject的属性
        if (propName == "objectName")
            continue;

        QString columnDef = propName + " ";

        // 根据类型添加字段类型
        QVariant::Type type = prop.type();
        if (type == QVariant::Int || type == QVariant::LongLong) {
            columnDef += "INTEGER";
        } else if (type == QVariant::Double) {
            columnDef += "REAL";
        } else if (type == QVariant::String) {
            columnDef += "TEXT";
        } else if (type == QVariant::Bool) {
            columnDef += "INTEGER";
        } else if (type == QVariant::ByteArray) {
            columnDef += "BLOB";
        } else {
            columnDef += "TEXT"; // 默认使用TEXT
        }

        // 添加字段属性
        FieldAttribute attr = m_fieldAttributes.value(propName);
        if (attr.isPrimaryKey) {
            columnDef += " PRIMARY KEY";
            if (attr.autoIncrement) {
                columnDef += " AUTOINCREMENT";
            }
        }
        if (attr.notNull) {
            columnDef += " NOT NULL";
        }
        if (attr.unique) {
            columnDef += " UNIQUE";
        }

        columns << columnDef;
    }

    return QString("CREATE TABLE IF NOT EXISTS %1 (%2);").arg(m_tableName).arg(columns.join(", "));
}

template<typename T>
bool GenericDao<T>::createTable()
{
    QSqlQuery query(m_db);
    QString   sql = buildCreateTableSQL();
    if (!query.exec(sql)) {
        m_lastError = "Create table failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }
    return true;
}

template<typename T>
QString GenericDao<T>::buildInsertSQL(T &entity)
{
    const QMetaObject *metaObj = &T::staticMetaObject;
    QStringList        fields;
    QStringList        placeholders;

    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        if (propName == "objectName")
            continue;

        // 如果是自增主键且值为0，则跳过
        if (m_fieldAttributes.value(propName).autoIncrement) {
            QVariant value = prop.readOnGadget(&entity);
            if (value.toInt() == 0)
                continue;
        }

        fields << propName;
        placeholders << ":" + propName;
    }

    return QString("INSERT INTO %1 (%2) VALUES (%3);")
        .arg(m_tableName)
        .arg(fields.join(", "))
        .arg(placeholders.join(", "));
}

template<typename T>
bool GenericDao<T>::insert(T &entity)
{
    QSqlQuery query(m_db);
    QString   sql = buildInsertSQL(entity);
    query.prepare(sql);

    const QMetaObject *metaObj = &T::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        if (propName == "objectName")
            continue;

        // 如果是自增主键且值为0，则跳过
        if (m_fieldAttributes.value(propName).autoIncrement) {
            QVariant value = prop.readOnGadget(&entity);
            if (value.toInt() == 0)
                continue;
        }

        QVariant value = prop.readOnGadget(&entity);
        query.bindValue(":" + propName, value);
    }

    if (!query.exec()) {
        m_lastError = "Insert failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // 如果是自增主键，设置新生成的ID
    if (!m_primaryKey.isEmpty() && m_fieldAttributes.value(m_primaryKey).autoIncrement) {
        QVariant id = query.lastInsertId();
        if (id.isValid()) {
            const QMetaObject *metaObj = &T::staticMetaObject;
            int                index = metaObj->indexOfProperty(m_primaryKey.toUtf8());
            if (index != -1) {
                QMetaProperty prop = metaObj->property(index);
                prop.writeOnGadget(&entity, id);
            }
        }
    }

    return true;
}

template<typename T>
QString GenericDao<T>::buildUpdateSQL(const T &entity)
{
    const QMetaObject *metaObj = &T::staticMetaObject;
    QStringList        setClauses;

    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        if (propName == "objectName" || propName == m_primaryKey)
            continue;

        setClauses << propName + " = :" + propName;
    }

    return QString("UPDATE %1 SET %2 WHERE %3 = :%3;").arg(m_tableName).arg(setClauses.join(", ")).arg(m_primaryKey);
}

template<typename T>
bool GenericDao<T>::update(const T &entity)
{
    QSqlQuery query(m_db);
    QString   sql = buildUpdateSQL(entity);
    query.prepare(sql);

    const QMetaObject *metaObj = &T::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        if (propName == "objectName")
            continue;

        QVariant value = prop.readOnGadget(&entity);
        query.bindValue(":" + propName, value);
    }

    if (!query.exec()) {
        m_lastError = "Update failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    return true;
}

template<typename T>
QString GenericDao<T>::buildDeleteSQL()
{
    return QString("DELETE FROM %1 WHERE %2 = :%2;").arg(m_tableName).arg(m_primaryKey);
}

template<typename T>
bool GenericDao<T>::remove(const T &entity)
{
    QSqlQuery query(m_db);
    QString   sql = buildDeleteSQL();
    query.prepare(sql);

    const QMetaObject *metaObj = &T::staticMetaObject;
    int                index = metaObj->indexOfProperty(m_primaryKey.toUtf8());
    if (index == -1) {
        m_lastError = "Primary key property not found";
        qWarning() << m_lastError;
        return false;
    }

    QMetaProperty prop = metaObj->property(index);
    QVariant      value = prop.readOnGadget(&entity);
    query.bindValue(":" + m_primaryKey, value);

    if (!query.exec()) {
        m_lastError = "Delete failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    return true;
}

template<typename T>
QString GenericDao<T>::buildSelectSQL()
{
    return QString("SELECT * FROM %1;").arg(m_tableName);
}

template<typename T>
QVector<T> GenericDao<T>::selectAll()
{
    QVector<T> results;
    QSqlQuery  query(m_db);
    QString    sql = buildSelectSQL();

    if (!query.exec(sql)) {
        m_lastError = "Select failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return results;
    }

    const QMetaObject *metaObj = &T::staticMetaObject;

    while (query.next()) {
        T entity;
        for (int i = 0; i < metaObj->propertyCount(); ++i) {
            QMetaProperty prop = metaObj->property(i);
            QString       propName = prop.name();

            if (propName == "objectName")
                continue;

            QVariant value = query.value(propName);
            if (value.isValid()) {
                prop.writeOnGadget(&entity, value);
            }
        }
        results.append(entity);
    }

    return results;
}

template<typename T>
T GenericDao<T>::selectById(QVariant id)
{
    T         entity;
    QSqlQuery query(m_db);
    QString   sql = QString("SELECT * FROM %1 WHERE %2 = ?;").arg(m_tableName).arg(m_primaryKey);

    query.prepare(sql);
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        m_lastError = "Select by ID failed: " + query.lastError().text();
        qWarning() << m_lastError;
        return entity;
    }

    const QMetaObject *metaObj = &T::staticMetaObject;
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        QString       propName = prop.name();

        if (propName == "objectName")
            continue;

        QVariant value = query.value(propName);
        if (value.isValid()) {
            prop.writeOnGadget(&entity, value);
        }
    }

    return entity;
}

template<typename T>
bool GenericDao<T>::beginTransaction()
{
    return m_db.transaction();
}

template<typename T>
bool GenericDao<T>::commitTransaction()
{
    return m_db.commit();
}

template<typename T>
bool GenericDao<T>::rollbackTransaction()
{
    return m_db.rollback();
}

// GenericDao 翻页方法实现
template<typename T>
QSharedPointer<PageResult<T>> GenericDao<T>::selectPage(int page, int pageSize)
{
    return query().paginate(page, pageSize);
}

template<typename T>
QSharedPointer<PageResult<T>> GenericDao<T>::selectPageWhere(const QString      &whereClause,
                                                             const QVariantList &bindValues,
                                                             int                 page,
                                                             int                 pageSize)
{
    QSharedPointer<PageResult<T>> result = QSharedPointer<PageResult<T>>::create();

    if (page < 1)
        page = 1;
    if (pageSize < 1)
        pageSize = 10;

    // 先获取总数
    QString   countSql = QString("SELECT COUNT(*) FROM %1 WHERE %2").arg(m_tableName).arg(whereClause);
    QSqlQuery countQuery(m_db);
    countQuery.prepare(countSql);
    for (const auto &value : bindValues) {
        countQuery.addBindValue(value);
    }

    int total = 0;
    if (countQuery.exec() && countQuery.next()) {
        total = countQuery.value(0).toInt();
    }

    // 计算翻页信息
    result->totalCount = total;
    result->currentPage = page;
    result->pageSize = pageSize;
    result->totalPages = (total + pageSize - 1) / pageSize;
    result->hasNext = page < result->totalPages;
    result->hasPrev = page > 1;

    // 如果没有数据，直接返回
    if (total == 0) {
        return result;
    }

    // 查询当前页数据
    int     offset = (page - 1) * pageSize;
    QString dataSql = QString("SELECT * FROM %1 WHERE %2 LIMIT %3 OFFSET %4")
                          .arg(m_tableName)
                          .arg(whereClause)
                          .arg(pageSize)
                          .arg(offset);

    result->data = executeSelectQuery(dataSql, bindValues);

    return result;
}

template<typename T>
QString GenericDao<T>::lastError() const
{
    return m_lastError;
}

} // namespace Database
} // namespace Core

#endif // GENERICDAO_H
