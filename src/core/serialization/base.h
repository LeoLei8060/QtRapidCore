#ifndef O_SERIALIZE_BASE_H
#define O_SERIALIZE_BASE_H

#include <array>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#ifdef O_SERIALIZE_USE_QT
#include <QHash>
#include <QList>
#include <QMap>
#include <QMultiHash>
#include <QMultiMap>
#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QScopedPointer>
#include <QSet>
#include <QSharedPointer>
#include <QStack>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QWeakPointer>
#endif

namespace OSerialize {

namespace Traits {
// 区分 STL 容器以便在序列化时统一处理为数组/列表结构
template<typename T>
struct is_stl_container : std::false_type
{};
template<typename T>
struct is_stl_container<std::vector<T>> : std::true_type
{};
template<typename T>
struct is_stl_container<std::list<T>> : std::true_type
{};
template<typename T>
struct is_stl_container<std::deque<T>> : std::true_type
{};
template<typename T>
struct is_stl_container<std::set<T>> : std::true_type
{};

// 区分 STL Map 以便在序列化时统一处理为键值对结构
template<typename T>
struct is_stl_map : std::false_type
{};
template<typename K, typename V>
struct is_stl_map<std::map<K, V>> : std::true_type
{};
template<typename K, typename V>
struct is_stl_map<std::unordered_map<K, V>> : std::true_type
{};
#ifdef O_SERIALIZE_USE_QT
// 允许 Qt 环境下的特化，但上面的通用模板已经覆盖了
#endif

// std::pair 序列化和反序列化时需要特殊处理，"first" 和 "second" 字段分别表示K和V
template<typename T>
struct is_pair : std::false_type
{};
template<typename K, typename V>
struct is_pair<std::pair<K, V>> : std::true_type
{};

template<typename T>
struct is_smart_ptr : std::false_type
{};
template<typename T>
struct is_smart_ptr<std::shared_ptr<T>> : std::true_type
{};
template<typename T>
struct is_smart_ptr<std::unique_ptr<T>> : std::true_type
{};

template<typename T>
struct is_variant : std::false_type
{};
template<typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type
{};

template<typename T>
struct is_tuple : std::false_type
{};
template<typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type
{};

#ifdef O_SERIALIZE_USE_QT
template<typename T>
struct is_qt_container : std::false_type
{};
template<typename T>
struct is_qt_container<QList<T>> : std::true_type
{};
template<typename T>
struct is_qt_container<QVector<T>> : std::true_type
{};
template<typename T>
struct is_qt_container<QSet<T>> : std::true_type
{};
template<typename T>
struct is_qt_container<QQueue<T>> : std::true_type
{};
template<typename T>
struct is_qt_container<QStack<T>> : std::true_type
{};
template<>
struct is_qt_container<QStringList> : std::true_type
{};

template<typename T>
struct is_qt_map : std::false_type
{};
template<typename K, typename V>
struct is_qt_map<QMap<K, V>> : std::true_type
{};
template<typename K, typename V>
struct is_qt_map<QHash<K, V>> : std::true_type
{};
// QMultiMap/QMultiHash 通常派生自 QMap/QHash，但在序列化时也作为 Map 处理
template<typename K, typename V>
struct is_qt_map<QMultiMap<K, V>> : std::true_type
{};
template<typename K, typename V>
struct is_qt_map<QMultiHash<K, V>> : std::true_type
{};

template<typename T>
struct is_qvariant : std::false_type
{};
template<>
struct is_qvariant<QVariant> : std::true_type
{};

template<typename T>
struct is_qpair : std::false_type
{};
template<typename T1, typename T2>
struct is_qpair<QPair<T1, T2>> : std::true_type
{};

template<typename T>
struct is_qt_smart_ptr : std::false_type
{};
template<typename T>
struct is_qt_smart_ptr<QSharedPointer<T>> : std::true_type
{};
template<typename T>
struct is_qt_smart_ptr<QWeakPointer<T>> : std::true_type
{}; // Weak pointers might need special handling (lock/toStrongRef)
template<typename T>
struct is_qt_smart_ptr<QPointer<T>> : std::true_type
{};
template<typename T>
struct is_qt_smart_ptr<QScopedPointer<T>> : std::true_type
{};
#endif
} // namespace Traits

} // namespace OSerialize

#endif // O_SERIALIZE_BASE_H
