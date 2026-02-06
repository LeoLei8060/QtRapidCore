#ifndef O_SERIALIZE_JSON_H
#define O_SERIALIZE_JSON_H

#include "base.h"
#include "o_serialize.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#ifdef O_SERIALIZE_USE_QT
#include <QByteArray>
#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QLine>
#include <QLineF>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QTime>
#endif

namespace OSerialize {

class JSON
{
public:
    template<typename T>
    static std::string obj_to_string(const T &obj)
    {
        rapidjson::Document doc;
        rapidjson::Value    val = to_json(obj, doc.GetAllocator());

        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        val.Accept(writer);
        return buffer.GetString();
    }

    template<typename T>
    static T string_to_obj(const std::string &json)
    {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        T obj;
        if (doc.HasParseError()) {
            std::cerr << "JSON Parse Error" << std::endl;
            return obj;
        }

        from_json(doc, obj);
        return obj;
    }

    template<typename T>
    static bool obj_to_file(const T &obj, const std::string &filepath)
    {
        rapidjson::Document doc;
        rapidjson::Value    val = to_json(obj, doc.GetAllocator());

        std::ofstream ofs(filepath);
        if (!ofs.is_open())
            return false;

        rapidjson::OStreamWrapper                          osw(ofs);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        val.Accept(writer);
        return true;
    }

    template<typename T>
    static T file_to_obj(const std::string &filepath)
    {
        T             obj;
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            std::cerr << "Cannot open file: " << filepath << std::endl;
            return obj;
        }

        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document       doc;
        doc.ParseStream(isw);

        if (doc.HasParseError()) {
            std::cerr << "JSON Parse Error" << std::endl;
            return obj;
        }

        from_json(doc, obj);
        return obj;
    }

private:
    template<typename C, typename V>
    static auto add_item(C &c, const V &v) -> decltype(c.push_back(v))
    {
        return c.push_back(v);
    }

    template<typename C, typename V>
    static auto add_item(C &c, const V &v) -> decltype(c.insert(v))
    {
        return c.insert(v);
    }

    template<typename T>
    static typename std::enable_if<Meta::has_reflection<T>::value, rapidjson::Value>::type to_json(
        const T &obj, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value json(rapidjson::kObjectType);
        Meta::visit_members(obj, [&](const char *name, const auto &member) {
            rapidjson::Value key(name, allocator);
            json.AddMember(key, to_json(member, allocator), allocator);
        });
        return json;
    }

    static rapidjson::Value to_json(short val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((int) val);
    }
    static rapidjson::Value to_json(unsigned short val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((unsigned int) val);
    }
    static rapidjson::Value to_json(int val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(unsigned int val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(long val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((int64_t) val);
    }
    static rapidjson::Value to_json(unsigned long val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((uint64_t) val);
    }
    static rapidjson::Value to_json(long long val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(unsigned long long val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(double val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(float val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }
    static rapidjson::Value to_json(bool val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value(val);
    }

    static rapidjson::Value to_json(signed char val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((int) val);
    }
    static rapidjson::Value to_json(unsigned char val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((unsigned int) val);
    }

    template<typename T>
    static typename std::enable_if<std::is_enum<T>::value, rapidjson::Value>::type to_json(
        T val, rapidjson::Document::AllocatorType &)
    {
        return rapidjson::Value((int) val);
    }

    static rapidjson::Value to_json(const std::string                  &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.c_str(), allocator);
    }

    static rapidjson::Value to_json(const char *val, rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val, allocator);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value, rapidjson::Value>::type to_json(
        const T &ptr, rapidjson::Document::AllocatorType &allocator)
    {
        if (!ptr)
            return rapidjson::Value(rapidjson::kNullType);
        return to_json(*ptr, allocator);
    }

    template<typename... Args>
    static rapidjson::Value to_json(const std::variant<Args...>        &v,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        return std::visit([&](const auto &val) { return to_json(val, allocator); }, v);
    }

    template<typename K, typename V>
    static rapidjson::Value to_json(const std::pair<K, V>              &pair,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        rapidjson::Value key1("first", allocator);
        obj.AddMember(key1, to_json(pair.first, allocator), allocator);
        rapidjson::Value key2("second", allocator);
        obj.AddMember(key2, to_json(pair.second, allocator), allocator);
        return obj;
    }

    template<typename K, typename V>
    static void from_json(const rapidjson::Value &json, std::pair<K, V> &pair)
    {
        if (!json.IsObject())
            return;
        if (json.HasMember("first"))
            from_json(json["first"], pair.first);
        if (json.HasMember("second"))
            from_json(json["second"], pair.second);
    }

    template<typename Tuple, size_t... Is>
    static void tuple_to_json_helper(const Tuple                        &t,
                                     rapidjson::Value                   &arr,
                                     rapidjson::Document::AllocatorType &allocator,
                                     std::index_sequence<Is...>)
    {
        (arr.PushBack(to_json(std::get<Is>(t), allocator), allocator), ...);
    }

    template<typename... Args>
    static
        typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value, rapidjson::Value>::type
        to_json(const std::tuple<Args...> &t, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value arr(rapidjson::kArrayType);
        tuple_to_json_helper(t, arr, allocator, std::index_sequence_for<Args...>{});
        return arr;
    }

#ifdef O_SERIALIZE_USE_QT
    static rapidjson::Value to_json(const QString                      &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.toStdString().c_str(), allocator);
    }

    static rapidjson::Value to_json(const QDate &val, rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.toString(Qt::ISODate).toStdString().c_str(), allocator);
    }
    static rapidjson::Value to_json(const QTime &val, rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.toString(Qt::ISODate).toStdString().c_str(), allocator);
    }
    static rapidjson::Value to_json(const QDateTime                    &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.toString(Qt::ISODate).toStdString().c_str(), allocator);
    }

    static rapidjson::Value to_json(const QPoint &val, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("x", val.x(), allocator);
        obj.AddMember("y", val.y(), allocator);
        return obj;
    }
    static rapidjson::Value to_json(const QPointF                      &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("x", val.x(), allocator);
        obj.AddMember("y", val.y(), allocator);
        return obj;
    }
    static rapidjson::Value to_json(const QSize &val, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("width", val.width(), allocator);
        obj.AddMember("height", val.height(), allocator);
        return obj;
    }
    static rapidjson::Value to_json(const QSizeF &val, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("width", val.width(), allocator);
        obj.AddMember("height", val.height(), allocator);
        return obj;
    }
    static rapidjson::Value to_json(const QRect &val, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("x", val.x(), allocator);
        obj.AddMember("y", val.y(), allocator);
        obj.AddMember("width", val.width(), allocator);
        obj.AddMember("height", val.height(), allocator);
        return obj;
    }
    static rapidjson::Value to_json(const QRectF &val, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("x", val.x(), allocator);
        obj.AddMember("y", val.y(), allocator);
        obj.AddMember("width", val.width(), allocator);
        obj.AddMember("height", val.height(), allocator);
        return obj;
    }

    static rapidjson::Value to_json(const QColor &val, rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.name().toStdString().c_str(), allocator);
    }

    static rapidjson::Value to_json(const QByteArray                   &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        return rapidjson::Value(val.toStdString().c_str(), allocator);
    }

    static rapidjson::Value to_json(const QVariant                     &val,
                                    rapidjson::Document::AllocatorType &allocator)
    {
        switch (val.type()) {
        case QVariant::Int:
            return to_json(val.toInt(), allocator);
        case QVariant::UInt:
            return to_json(val.toUInt(), allocator);
        case QVariant::LongLong:
            return to_json(val.toLongLong(), allocator);
        case QVariant::ULongLong:
            return to_json(val.toULongLong(), allocator);
        case QVariant::Double:
            return to_json(val.toDouble(), allocator);
        case QVariant::Bool:
            return to_json(val.toBool(), allocator);
        case QVariant::String:
            return to_json(val.toString(), allocator);
        case QVariant::StringList:
            return to_json(val.toStringList(), allocator);
        case QVariant::List:
            return to_json(val.toList(), allocator);
        case QVariant::Map:
            return to_json(val.toMap(), allocator);
        case QVariant::Hash:
            return to_json(val.toHash(), allocator);
        default:
            // 如果可能，尝试转换为字符串
            if (val.canConvert<QString>())
                return to_json(val.toString(), allocator);
            return rapidjson::Value(rapidjson::kNullType);
        }
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value, rapidjson::Value>::type to_json(
        const T &ptr, rapidjson::Document::AllocatorType &allocator)
    {
        if (ptr.isNull())
            return rapidjson::Value(rapidjson::kNullType);
        return to_json(*ptr, allocator);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value>::type from_json(
        const rapidjson::Value &json, T &ptr)
    {
        if (json.IsNull()) {
            ptr.reset();
            return;
        }
        create_qt_smart_ptr(ptr, json);
    }

    template<typename T>
    static auto create_qt_smart_ptr(QSharedPointer<T> &ptr, const rapidjson::Value &json) -> void
    {
        ptr = QSharedPointer<T>::create();
        from_json(json, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr(QScopedPointer<T> &ptr, const rapidjson::Value &json) -> void
    {
        ptr.reset(new T());
        from_json(json, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr(QPointer<T> &ptr, const rapidjson::Value &) -> void
    {
        // WARNING：QPointer 反序列化未完全支持（所有权问题）。
        ptr = nullptr;
    }

#endif
    template<typename T>
    static typename std::enable_if<Traits::is_stl_container<T>::value
                                       || Traits::is_qt_container<T>::value,
                                   rapidjson::Value>::type
    to_json(const T &container, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value arr(rapidjson::kArrayType);
        for (const auto &item : container) {
            arr.PushBack(to_json(item, allocator), allocator);
        }
        return arr;
    }

    template<typename T>
    static typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value,
                                   rapidjson::Value>::type
    to_json(const T &map, rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value obj(rapidjson::kObjectType);

        if constexpr (Traits::is_stl_map<T>::value) {
            using KeyType = typename T::key_type;

#ifdef O_SERIALIZE_USE_QT
            constexpr bool is_std_string = std::is_same<KeyType, std::string>::value;
            constexpr bool is_qstring = std::is_same<KeyType, QString>::value;
            // JSON 对象的键必须是字符串。如果 Map 的键不是字符串，这里会尝试转换。
            static_assert(is_std_string || is_qstring, "STL Map key must be std::string or QString");
#else
            constexpr bool is_std_string = std::is_same<KeyType, std::string>::value;
            static_assert(is_std_string, "STL Map key must be std::string");
#endif

            for (const auto &pair : map) {
                if constexpr (is_std_string) {
                    rapidjson::Value key(pair.first.c_str(), allocator);
                    obj.AddMember(key, to_json(pair.second, allocator), allocator);
                }
#ifdef O_SERIALIZE_USE_QT
                else if constexpr (is_qstring) {
                    std::string      keyStr = pair.first.toStdString();
                    rapidjson::Value key(keyStr.c_str(), allocator);
                    obj.AddMember(key, to_json(pair.second, allocator), allocator);
                }
#endif
            }
        }
#ifdef O_SERIALIZE_USE_QT
        else if constexpr (Traits::is_qt_map<T>::value) {
            auto it = map.begin();
            while (it != map.end()) {
                rapidjson::Value keyVal;
                // 处理 QString 键转换
                // 假设键是 QString 或可转换为 string
                std::string keyStr = val_to_string_helper(it.key());
                keyVal.SetString(keyStr.c_str(), allocator);

                obj.AddMember(keyVal, to_json(it.value(), allocator), allocator);
                ++it;
            }
        }
#endif
        return obj;
    }

    template<typename K>
    static std::string val_to_string_helper(const K &key)
    {
        return std::to_string(key);
    }
    static std::string val_to_string_helper(const std::string &key) { return key; }
#ifdef O_SERIALIZE_USE_QT
    static std::string val_to_string_helper(const QString &key) { return key.toStdString(); }
#endif

    template<typename T>
    static typename std::enable_if<Meta::has_reflection<T>::value>::type from_json(
        const rapidjson::Value &json, T &obj)
    {
        if (!json.IsObject())
            return;
        Meta::visit_members(obj, [&](const char *name, auto &member) {
            if (json.HasMember(name)) {
                from_json(json[name], member);
            }
        });
    }

    static void from_json(const rapidjson::Value &json, short &val)
    {
        if (json.IsInt())
            val = (short) json.GetInt();
    }
    static void from_json(const rapidjson::Value &json, unsigned short &val)
    {
        if (json.IsUint())
            val = (unsigned short) json.GetUint();
    }
    static void from_json(const rapidjson::Value &json, int &val)
    {
        if (json.IsInt())
            val = json.GetInt();
    }
    static void from_json(const rapidjson::Value &json, unsigned int &val)
    {
        if (json.IsUint())
            val = json.GetUint();
    }
    static void from_json(const rapidjson::Value &json, long &val)
    {
        if (json.IsInt64())
            val = (long) json.GetInt64();
    }
    static void from_json(const rapidjson::Value &json, unsigned long &val)
    {
        if (json.IsUint64())
            val = (unsigned long) json.GetUint64();
    }
    static void from_json(const rapidjson::Value &json, long long &val)
    {
        if (json.IsInt64())
            val = json.GetInt64();
    }
    static void from_json(const rapidjson::Value &json, unsigned long long &val)
    {
        if (json.IsUint64())
            val = json.GetUint64();
    }
    static void from_json(const rapidjson::Value &json, double &val)
    {
        if (json.IsNumber())
            val = json.GetDouble();
    }
    static void from_json(const rapidjson::Value &json, float &val)
    {
        if (json.IsNumber())
            val = json.GetFloat();
    }
    static void from_json(const rapidjson::Value &json, bool &val)
    {
        if (json.IsBool())
            val = json.GetBool();
    }

    static void from_json(const rapidjson::Value &json, signed char &val)
    {
        if (json.IsInt())
            val = (signed char) json.GetInt();
    }
    static void from_json(const rapidjson::Value &json, unsigned char &val)
    {
        if (json.IsUint())
            val = (unsigned char) json.GetUint();
    }

    template<typename T>
    static typename std::enable_if<std::is_enum<T>::value>::type from_json(
        const rapidjson::Value &json, T &val)
    {
        if (json.IsInt())
            val = static_cast<T>(json.GetInt());
    }

    static void from_json(const rapidjson::Value &json, std::string &val)
    {
        if (json.IsString())
            val = json.GetString();
    }

    template<typename Tuple, size_t... Is>
    static void tuple_from_json_helper(const rapidjson::Value &arr,
                                       Tuple                  &t,
                                       std::index_sequence<Is...>)
    {
        if (!arr.IsArray())
            return;
        size_t size = arr.Size();
        ((Is < size ? (from_json(arr[Is], std::get<Is>(t)), 0) : 0), ...);
    }

    template<typename... Args>
    static typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value>::type from_json(
        const rapidjson::Value &json, std::tuple<Args...> &t)
    {
        tuple_from_json_helper(json, t, std::index_sequence_for<Args...>{});
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value>::type from_json(
        const rapidjson::Value &json, T &ptr)
    {
        if (json.IsNull()) {
            ptr.reset();
            return;
        }
        create_smart_ptr(ptr);
        from_json(json, *ptr);
    }

    template<typename T>
    static void create_smart_ptr(std::shared_ptr<T> &ptr)
    {
        ptr = std::make_shared<T>();
    }

    template<typename T>
    static void create_smart_ptr(std::unique_ptr<T> &ptr)
    {
        ptr = std::unique_ptr<T>(new T());
    }

#ifdef O_SERIALIZE_USE_QT
    static void from_json(const rapidjson::Value &json, QString &val)
    {
        if (json.IsString())
            val = QString::fromUtf8(json.GetString());
    }

    static void from_json(const rapidjson::Value &json, QDate &val)
    {
        if (json.IsString())
            val = QDate::fromString(QString::fromUtf8(json.GetString()), Qt::ISODate);
    }
    static void from_json(const rapidjson::Value &json, QTime &val)
    {
        if (json.IsString())
            val = QTime::fromString(QString::fromUtf8(json.GetString()), Qt::ISODate);
    }
    static void from_json(const rapidjson::Value &json, QDateTime &val)
    {
        if (json.IsString())
            val = QDateTime::fromString(QString::fromUtf8(json.GetString()), Qt::ISODate);
    }

    static void from_json(const rapidjson::Value &json, QPoint &val)
    {
        if (json.IsObject()) {
            if (json.HasMember("x"))
                val.setX(json["x"].GetInt());
            if (json.HasMember("y"))
                val.setY(json["y"].GetInt());
        }
    }
    static void from_json(const rapidjson::Value &json, QPointF &val)
    {
        if (json.IsObject()) {
            if (json.HasMember("x"))
                val.setX(json["x"].GetDouble());
            if (json.HasMember("y"))
                val.setY(json["y"].GetDouble());
        }
    }
    static void from_json(const rapidjson::Value &json, QSize &val)
    {
        if (json.IsObject()) {
            if (json.HasMember("width"))
                val.setWidth(json["width"].GetInt());
            if (json.HasMember("height"))
                val.setHeight(json["height"].GetInt());
        }
    }
    static void from_json(const rapidjson::Value &json, QSizeF &val)
    {
        if (json.IsObject()) {
            if (json.HasMember("width"))
                val.setWidth(json["width"].GetDouble());
            if (json.HasMember("height"))
                val.setHeight(json["height"].GetDouble());
        }
    }
    static void from_json(const rapidjson::Value &json, QRect &val)
    {
        if (json.IsObject()) {
            int x = 0, y = 0, w = 0, h = 0;
            if (json.HasMember("x"))
                x = json["x"].GetInt();
            if (json.HasMember("y"))
                y = json["y"].GetInt();
            if (json.HasMember("width"))
                w = json["width"].GetInt();
            if (json.HasMember("height"))
                h = json["height"].GetInt();
            val.setRect(x, y, w, h);
        }
    }
    static void from_json(const rapidjson::Value &json, QRectF &val)
    {
        if (json.IsObject()) {
            double x = 0, y = 0, w = 0, h = 0;
            if (json.HasMember("x"))
                x = json["x"].GetDouble();
            if (json.HasMember("y"))
                y = json["y"].GetDouble();
            if (json.HasMember("width"))
                w = json["width"].GetDouble();
            if (json.HasMember("height"))
                h = json["height"].GetDouble();
            val.setRect(x, y, w, h);
        }
    }

    static void from_json(const rapidjson::Value &json, QColor &val)
    {
        if (json.IsString())
            val.setNamedColor(QString::fromUtf8(json.GetString()));
    }

    static void from_json(const rapidjson::Value &json, QByteArray &val)
    {
        if (json.IsString())
            val = QByteArray(json.GetString());
    }

    template<typename T>
    static void from_json(const rapidjson::Value &json, QSharedPointer<T> &ptr)
    {
        if (json.IsNull()) {
            ptr.reset();
            return;
        }
        ptr = QSharedPointer<T>::create();
        from_json(json, *ptr);
    }

    template<typename T>
    static void from_json(const rapidjson::Value &json, QScopedPointer<T> &ptr)
    {
        if (json.IsNull()) {
            ptr.reset();
            return;
        }
        ptr.reset(new T());
        from_json(json, *ptr);
    }

    template<typename T>
    static void from_json(const rapidjson::Value &, QPointer<T> &ptr)
    {
        ptr = nullptr;
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qpair<T>::value>::type from_json(
        const rapidjson::Value &json, T &pair)
    {
        if (!json.IsObject())
            return;
        if (json.HasMember("first"))
            from_json(json["first"], pair.first);
        if (json.HasMember("second"))
            from_json(json["second"], pair.second);
    }

    static void from_json(const rapidjson::Value &json, QVariant &val)
    {
        if (json.IsInt())
            val = json.GetInt();
        else if (json.IsUint())
            val = json.GetUint();
        else if (json.IsInt64())
            val = (qlonglong) json.GetInt64();
        else if (json.IsUint64())
            val = (qulonglong) json.GetUint64();
        else if (json.IsDouble())
            val = json.GetDouble();
        else if (json.IsBool())
            val = json.GetBool();
        else if (json.IsString())
            val = QString::fromUtf8(json.GetString());
        else if (json.IsArray()) {
            QList<QVariant> list;
            from_json(json, list);
            val = list;
        } else if (json.IsObject()) {
            QMap<QString, QVariant> map;
            from_json(json, map);
            val = map;
        } else
            val = QVariant();
    }
#endif

    template<typename T>
    static typename std::enable_if<Traits::is_stl_container<T>::value
                                   || Traits::is_qt_container<T>::value>::type
    from_json(const rapidjson::Value &json, T &container)
    {
        if (!json.IsArray())
            return;
        container.clear();
        for (const auto &item : json.GetArray()) {
            typename T::value_type val;
            from_json(item, val);
            add_item(container, val);
        }
    }

    template<typename T>
    static
        typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value>::type
        from_json(const rapidjson::Value &json, T &map)
    {
        if (!json.IsObject())
            return;

        map.clear();

        if constexpr (Traits::is_stl_map<T>::value) {
            using KeyType = typename T::key_type;

#ifdef O_SERIALIZE_USE_QT
            constexpr bool is_std_string = std::is_same<KeyType, std::string>::value;
            constexpr bool is_qstring = std::is_same<KeyType, QString>::value;
            static_assert(is_std_string || is_qstring, "STL Map key must be std::string or QString");
#else
            constexpr bool is_std_string = std::is_same<KeyType, std::string>::value;
            static_assert(is_std_string, "STL Map key must be std::string");
#endif

            for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
                typename T::mapped_type val;
                from_json(it->value, val);

                if constexpr (is_std_string) {
                    map[it->name.GetString()] = val;
                }
#ifdef O_SERIALIZE_USE_QT
                else if constexpr (is_qstring) {
                    map[QString::fromUtf8(it->name.GetString())] = val;
                }
#endif
            }
        } else {
            for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
                typename T::mapped_type val;
                from_json(it->value, val);
                insert_map_item(map, it->name.GetString(), val);
            }
        }
    }

    template<typename T>
    static T key_from_string(const char *key)
    {
        if constexpr (std::is_integral<T>::value)
            return static_cast<T>(std::atoll(key));
        else if constexpr (std::is_floating_point<T>::value)
            return static_cast<T>(std::atof(key));
        else
            return T(key);
    }

    template<typename Map, typename Val>
    static void insert_map_item(Map &map, const char *key, const Val &val)
    {
        map[key_from_string<typename Map::key_type>(key)] = val;
    }
};

} // namespace OSerialize

#endif // O_SERIALIZE_JSON_H
