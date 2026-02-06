#ifndef O_SERIALIZE_INI_H
#define O_SERIALIZE_INI_H

#include "base.h"
#include "inipp/inipp.h"
#include "o_serialize.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef O_SERIALIZE_USE_QT
#include <QByteArray>
#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QStringList>
#include <QTime>
#include <QVariant>
#include <QVector>
#endif

namespace OSerialize {

class INI
{
public:
    template<typename T>
    static std::string obj_to_string(const T &obj, const std::string &sectionName = "default")
    {
        inipp::Ini<char> ini;
        to_ini(obj, ini.sections[sectionName]);

        std::stringstream ss;
        ini.generate(ss);
        return ss.str();
    }

    template<typename T>
    static T string_to_obj(const std::string &iniStr, const std::string &sectionName = "default")
    {
        inipp::Ini<char>   ini;
        std::istringstream is(iniStr);
        ini.parse(is);

        T obj;
        if (ini.sections.find(sectionName) == ini.sections.end()) {
            return obj;
        }

        from_ini(ini.sections[sectionName], obj);
        return obj;
    }

    template<typename T>
    static bool obj_to_file(const T           &obj,
                            const std::string &filepath,
                            const std::string &sectionName = "default")
    {
        std::ofstream ofs(filepath);
        if (!ofs.is_open())
            return false;

        inipp::Ini<char> ini;
        to_ini(obj, ini.sections[sectionName]);

        ini.generate(ofs);
        return true;
    }

    template<typename T>
    static T file_to_obj(const std::string &filepath, const std::string &sectionName = "default")
    {
        T             obj;
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            std::cerr << "Cannot open file: " << filepath << std::endl;
            return obj;
        }

        inipp::Ini<char> ini;
        ini.parse(ifs);

        if (ini.sections.find(sectionName) == ini.sections.end()) {
            return obj;
        }

        from_ini(ini.sections[sectionName], obj);
        return obj;
    }

private:
    template<typename T>
    static typename std::enable_if<Meta::has_reflection<T>::value, void>::type to_ini(
        const T &obj, inipp::Ini<char>::Section &section)
    {
        Meta::visit_members(obj, [&](const char *name, const auto &member) {
            section[name] = val_to_string(member);
        });
    }

    template<typename T>
    static
        typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, void>::type
        to_ini(const T &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }

    static void to_ini(const std::string &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val;
    }

    static void to_ini(const char *val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val;
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value, void>::type to_ini(
        const T &ptr, inipp::Ini<char>::Section &section)
    {
        if (ptr) {
            to_ini(*ptr, section);
        } else {
            section["null"] = "true";
        }
    }

    template<typename K, typename V>
    static void to_ini(const std::pair<K, V> &pair, inipp::Ini<char>::Section &section)
    {
        section["first"] = val_to_string(pair.first);
        section["second"] = val_to_string(pair.second);
    }

    template<typename... Args>
    static void to_ini(const std::variant<Args...> &v, inipp::Ini<char>::Section &section)
    {
        std::visit([&](const auto &val) { to_ini(val, section); }, v);
    }

    template<typename Tuple, size_t... Is>
    static void tuple_to_ini_helper(const Tuple               &t,
                                    inipp::Ini<char>::Section &section,
                                    std::index_sequence<Is...>)
    {
        ([&]() { section["item" + std::to_string(Is)] = val_to_string(std::get<Is>(t)); }(), ...);
    }

    template<typename... Args>
    static typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value, void>::type to_ini(
        const std::tuple<Args...> &t, inipp::Ini<char>::Section &section)
    {
        tuple_to_ini_helper(t, section, std::index_sequence_for<Args...>{});
    }

#ifdef O_SERIALIZE_USE_QT
    static void to_ini(const QString &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val.toStdString();
    }

    // Qt Date/Time/Geometry -> to string value
    static void to_ini(const QDate &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QTime &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QDateTime &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QPoint &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QPointF &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QSize &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QSizeF &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QRect &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QRectF &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QColor &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val_to_string(val);
    }
    static void to_ini(const QByteArray &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val.toStdString();
    }

    static void to_ini(const QVariant &val, inipp::Ini<char>::Section &section)
    {
        section["value"] = val.toString().toStdString();
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value, void>::type to_ini(
        const T &ptr, inipp::Ini<char>::Section &section)
    {
        if (ptr) {
            to_ini(*ptr, section);
        } else {
            section["null"] = "true";
        }
    }
#endif

    template<typename T>
    static typename std::enable_if<Traits::is_stl_map<T>::value, void>::type to_ini(
        const T &map, inipp::Ini<char>::Section &section)
    {
        for (const auto &pair : map) {
            section[val_to_string(pair.first)] = val_to_string(pair.second);
        }
    }

#ifdef O_SERIALIZE_USE_QT
    template<typename T>
    static typename std::enable_if<Traits::is_qt_map<T>::value, void>::type to_ini(
        const T &map, inipp::Ini<char>::Section &section)
    {
        auto it = map.begin();
        while (it != map.end()) {
            section[val_to_string(it.key())] = val_to_string(it.value());
            ++it;
        }
    }
#endif

    template<typename T>
    static
        typename std::enable_if<!Traits::is_stl_map<T>::value && !std::is_same<T, std::string>::value
                                    && !Traits::is_pair<T>::value && !Traits::is_tuple<T>::value
                                    && !Traits::is_smart_ptr<T>::value
#ifdef O_SERIALIZE_USE_QT
                                    && !Traits::is_qt_map<T>::value && !Traits::is_qpair<T>::value
                                    && !Traits::is_qt_smart_ptr<T>::value
#endif
                                ,
                                std::string>::type
        val_to_string(const T &val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    static std::string val_to_string(const std::string &val) { return val; }
    static std::string val_to_string(const char *val) { return std::string(val); }

    template<typename T>
    static typename std::enable_if<Traits::is_pair<T>::value, std::string>::type val_to_string(
        const T &pair)
    {
        return "(" + val_to_string(pair.first) + ", " + val_to_string(pair.second) + ")";
    }

#ifdef O_SERIALIZE_USE_QT
    template<typename T>
    static typename std::enable_if<Traits::is_qpair<T>::value, std::string>::type val_to_string(
        const T &pair)
    {
        return "(" + val_to_string(pair.first) + ", " + val_to_string(pair.second) + ")";
    }
#endif

    template<typename Tuple, size_t... Is>
    static void tuple_to_string_helper(std::stringstream &ss,
                                       const Tuple       &t,
                                       std::index_sequence<Is...>)
    {
        bool first = true;
        (
            [&]() {
                if (!first)
                    ss << ", ";
                ss << val_to_string(std::get<Is>(t));
                first = false;
            }(),
            ...);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_tuple<T>::value, std::string>::type val_to_string(
        const T &t)
    {
        std::stringstream ss;
        ss << "(";
        tuple_to_string_helper(ss, t, std::make_index_sequence<std::tuple_size<T>::value>{});
        ss << ")";
        return ss.str();
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value, std::string>::type val_to_string(
        const T &ptr)
    {
        if (!ptr)
            return "null";
        return val_to_string(*ptr);
    }

#ifdef O_SERIALIZE_USE_QT
    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value, std::string>::type
    val_to_string(const T &ptr)
    {
        if (!ptr)
            return "null";
        return val_to_string(*ptr);
    }
#endif

    template<typename T>
    static typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value,
                                   std::string>::type
    val_to_string(const T &map)
    {
        std::stringstream ss;
        ss << "{";
        bool first = true;

        auto serialize_pair = [&](const std::string &k, const std::string &v) {
            if (!first)
                ss << ", ";
            ss << k << ":" << v;
            first = false;
        };

        if constexpr (Traits::is_stl_map<T>::value) {
            for (const auto &p : map)
                serialize_pair(val_to_string(p.first), val_to_string(p.second));
        }
#ifdef O_SERIALIZE_USE_QT
        else if constexpr (Traits::is_qt_map<T>::value) {
            auto it = map.begin();
            while (it != map.end()) {
                serialize_pair(val_to_string(it.key()), val_to_string(it.value()));
                ++it;
            }
        }
#endif
        ss << "}";
        return ss.str();
    }

#ifdef O_SERIALIZE_USE_QT
    static std::string val_to_string(const QString &val) { return val.toStdString(); }
    static std::string val_to_string(const QDate &val)
    {
        return val.toString(Qt::ISODate).toStdString();
    }
    static std::string val_to_string(const QTime &val)
    {
        return val.toString(Qt::ISODate).toStdString();
    }
    static std::string val_to_string(const QDateTime &val)
    {
        return val.toString(Qt::ISODate).toStdString();
    }
    static std::string val_to_string(const QPoint &val)
    {
        return std::to_string(val.x()) + "," + std::to_string(val.y());
    }
    static std::string val_to_string(const QPointF &val)
    {
        return std::to_string(val.x()) + "," + std::to_string(val.y());
    }
    static std::string val_to_string(const QSize &val)
    {
        return std::to_string(val.width()) + "," + std::to_string(val.height());
    }
    static std::string val_to_string(const QSizeF &val)
    {
        return std::to_string(val.width()) + "," + std::to_string(val.height());
    }
    static std::string val_to_string(const QRect &val)
    {
        return std::to_string(val.x()) + "," + std::to_string(val.y()) + ","
               + std::to_string(val.width()) + "," + std::to_string(val.height());
    }
    static std::string val_to_string(const QRectF &val)
    {
        return std::to_string(val.x()) + "," + std::to_string(val.y()) + ","
               + std::to_string(val.width()) + "," + std::to_string(val.height());
    }
    static std::string val_to_string(const QColor &val) { return val.name().toStdString(); }
    static std::string val_to_string(const QByteArray &val) { return val.toStdString(); }
    static std::string val_to_string(const QVariant &val) { return val.toString().toStdString(); }
#endif

    template<typename T>
    static typename std::enable_if<Meta::has_reflection<T>::value>::type from_ini(
        const inipp::Ini<char>::Section &section, T &obj)
    {
        Meta::visit_members(obj, [&](const char *name, auto &member) {
            auto it = section.find(name);
            if (it != section.end()) {
                string_to_val(it->second, member);
            }
        });
    }

    template<typename T>
    static
        typename std::enable_if<!Traits::is_stl_map<T>::value && !std::is_same<T, std::string>::value
                                    && !Traits::is_tuple<T>::value && !Traits::is_pair<T>::value
                                    && !Traits::is_smart_ptr<T>::value
#ifdef O_SERIALIZE_USE_QT
                                    && !Traits::is_qt_map<T>::value
                                    && !Traits::is_qt_smart_ptr<T>::value && !Traits::is_qpair<T>::value
#endif
                                ,
                                void>::type
        string_to_val(const std::string &s, T &val)
    {
        std::stringstream ss(s);
        ss >> val;
    }

    static void string_to_val(const std::string &s, std::string &val) { val = s; }

    template<typename T>
    static typename std::enable_if<Traits::is_pair<T>::value, void>::type string_to_val(
        const std::string &s, T &pair)
    {
        std::string content = s;
        if (content.size() >= 2 && content.front() == '(' && content.back() == ')') {
            content = content.substr(1, content.size() - 2);
        }
        size_t comma = content.find(',');
        if (comma != std::string::npos) {
            std::string s1 = content.substr(0, comma);
            std::string s2 = content.substr(comma + 1);
            auto        trim = [](std::string &str) {
                size_t first = str.find_first_not_of(' ');
                if (first == std::string::npos) {
                    str.clear();
                    return;
                }
                size_t last = str.find_last_not_of(' ');
                str = str.substr(first, (last - first + 1));
            };
            trim(s1);
            trim(s2);
            string_to_val(s1, pair.first);
            string_to_val(s2, pair.second);
        }
    }

#ifdef O_SERIALIZE_USE_QT
    template<typename T>
    static typename std::enable_if<Traits::is_qpair<T>::value, void>::type string_to_val(
        const std::string &s, T &pair)
    {
        std::string content = s;
        if (content.size() >= 2 && content.front() == '(' && content.back() == ')') {
            content = content.substr(1, content.size() - 2);
        }
        size_t comma = content.find(',');
        if (comma != std::string::npos) {
            std::string s1 = content.substr(0, comma);
            std::string s2 = content.substr(comma + 1);
            auto        trim = [](std::string &str) {
                size_t first = str.find_first_not_of(' ');
                if (first == std::string::npos) {
                    str.clear();
                    return;
                }
                size_t last = str.find_last_not_of(' ');
                str = str.substr(first, (last - first + 1));
            };
            trim(s1);
            trim(s2);
            string_to_val(s1, pair.first);
            string_to_val(s2, pair.second);
        }
    }
#endif

    template<typename Tuple, size_t... Is>
    static void tuple_from_string_helper(const std::vector<std::string> &parts,
                                         Tuple                          &t,
                                         std::index_sequence<Is...>)
    {
        (
            [&]() {
                if (Is < parts.size()) {
                    string_to_val(parts[Is], std::get<Is>(t));
                }
            }(),
            ...);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_tuple<T>::value, void>::type string_to_val(
        const std::string &s, T &t)
    {
        std::string content = s;
        if (content.size() >= 2 && content.front() == '(' && content.back() == ')') {
            content = content.substr(1, content.size() - 2);
        }
        std::vector<std::string> parts;
        std::stringstream        ss(content);
        std::string              segment;
        auto                     trim = [](std::string &str) {
            size_t first = str.find_first_not_of(' ');
            if (first == std::string::npos) {
                str.clear();
                return;
            }
            size_t last = str.find_last_not_of(' ');
            str = str.substr(first, (last - first + 1));
        };
        while (std::getline(ss, segment, ',')) {
            trim(segment);
            parts.push_back(segment);
        }
        tuple_from_string_helper(parts, t, std::make_index_sequence<std::tuple_size<T>::value>{});
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value, void>::type string_to_val(
        const std::string &s, T &ptr)
    {
        if (s == "null" || s == "" || s == "0" || s == "false") {
            ptr.reset();
            return;
        }
        using Elem = typename T::element_type;
        ptr = std::make_shared<Elem>();
        string_to_val(s, *ptr);
    }

#ifdef O_SERIALIZE_USE_QT
    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value, void>::type string_to_val(
        const std::string &s, T &ptr)
    {
        if (s == "null" || s == "" || s == "0" || s == "false") {
            ptr.reset();
            return;
        }
        using Elem = typename T::Type;
        ptr = QSharedPointer<Elem>::create();
        string_to_val(s, *ptr);
    }
#endif

    template<typename T>
    static
        typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value, void>::type
        string_to_val(const std::string &s, T &map)
    {
        map.clear();
        std::string content = s;
        if (content.size() >= 2 && content.front() == '{' && content.back() == '}') {
            content = content.substr(1, content.size() - 2);
        }

        std::stringstream ss(content);
        std::string       segment;
        while (std::getline(ss, segment, ',')) {
            size_t colon = segment.find(':');
            if (colon != std::string::npos) {
                std::string kStr = segment.substr(0, colon);
                std::string vStr = segment.substr(colon + 1);

                kStr.erase(0, kStr.find_first_not_of(" "));
                vStr.erase(0, vStr.find_first_not_of(" "));

                typename T::key_type    key;
                typename T::mapped_type val;
                string_to_val(kStr, key);
                string_to_val(vStr, val);

#ifdef O_SERIALIZE_USE_QT
                if constexpr (Traits::is_qt_map<T>::value)
                    map.insert(key, val);
                else
#endif
                    map[key] = val;
            }
        }
    }

#ifdef O_SERIALIZE_USE_QT
    static void string_to_val(const std::string &s, QString &val)
    {
        val = QString::fromStdString(s);
    }
    static void string_to_val(const std::string &s, QDate &val)
    {
        val = QDate::fromString(QString::fromStdString(s), Qt::ISODate);
    }
    static void string_to_val(const std::string &s, QTime &val)
    {
        val = QTime::fromString(QString::fromStdString(s), Qt::ISODate);
    }
    static void string_to_val(const std::string &s, QDateTime &val)
    {
        val = QDateTime::fromString(QString::fromStdString(s), Qt::ISODate);
    }

    static void string_to_val(const std::string &s, QPoint &val)
    {
        size_t comma = s.find(',');
        if (comma != std::string::npos) {
            val.setX(std::stoi(s.substr(0, comma)));
            val.setY(std::stoi(s.substr(comma + 1)));
        }
    }
    static void string_to_val(const std::string &s, QPointF &val)
    {
        size_t comma = s.find(',');
        if (comma != std::string::npos) {
            val.setX(std::stod(s.substr(0, comma)));
            val.setY(std::stod(s.substr(comma + 1)));
        }
    }
    static void string_to_val(const std::string &s, QSize &val)
    {
        size_t comma = s.find(',');
        if (comma != std::string::npos) {
            val.setWidth(std::stoi(s.substr(0, comma)));
            val.setHeight(std::stoi(s.substr(comma + 1)));
        }
    }
    static void string_to_val(const std::string &s, QSizeF &val)
    {
        size_t comma = s.find(',');
        if (comma != std::string::npos) {
            val.setWidth(std::stod(s.substr(0, comma)));
            val.setHeight(std::stod(s.substr(comma + 1)));
        }
    }

    static void string_to_val(const std::string &s, QRect &val)
    {
        std::stringstream ss(s);
        std::string       seg;
        std::vector<int>  v;
        while (std::getline(ss, seg, ','))
            v.push_back(std::stoi(seg));
        if (v.size() >= 4)
            val.setRect(v[0], v[1], v[2], v[3]);
    }
    static void string_to_val(const std::string &s, QRectF &val)
    {
        std::stringstream   ss(s);
        std::string         seg;
        std::vector<double> v;
        while (std::getline(ss, seg, ','))
            v.push_back(std::stod(seg));
        if (v.size() >= 4)
            val.setRect(v[0], v[1], v[2], v[3]);
    }

    static void string_to_val(const std::string &s, QColor &val)
    {
        val.setNamedColor(QString::fromStdString(s));
    }
    static void string_to_val(const std::string &s, QByteArray &val)
    {
        val = QByteArray::fromStdString(s);
    }
    static void string_to_val(const std::string &s, QVariant &val)
    {
        val = QString::fromStdString(s);
    }
#endif

    template<typename T>
    static
        typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, void>::type
        from_ini(const inipp::Ini<char>::Section &section, T &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }

    static void from_ini(const inipp::Ini<char>::Section &section, std::string &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            val = it->second;
    }

#ifdef O_SERIALIZE_USE_QT
    static void from_ini(const inipp::Ini<char>::Section &section, QString &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            val = QString::fromStdString(it->second);
    }

    static void from_ini(const inipp::Ini<char>::Section &section, QPoint &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QPointF &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QSize &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QSizeF &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QRect &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QRectF &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QDate &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QTime &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QDateTime &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QColor &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QByteArray &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }
    static void from_ini(const inipp::Ini<char>::Section &section, QVariant &val)
    {
        auto it = section.find("value");
        if (it != section.end())
            string_to_val(it->second, val);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value>::type from_ini(
        const inipp::Ini<char>::Section &section, T &ptr)
    {
        auto it = section.find("null");
        if (it != section.end() && (it->second == "true" || it->second == "1")) {
            ptr.reset();
            return;
        }
        create_qt_smart_ptr_ini(ptr, section);
    }

    template<typename T>
    static auto create_qt_smart_ptr_ini(QSharedPointer<T>               &ptr,
                                        const inipp::Ini<char>::Section &section) -> void
    {
        ptr = QSharedPointer<T>::create();
        from_ini(section, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr_ini(QScopedPointer<T>               &ptr,
                                        const inipp::Ini<char>::Section &section) -> void
    {
        ptr.reset(new T());
        from_ini(section, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr_ini(QPointer<T> &ptr, const inipp::Ini<char>::Section &) -> void
    {
        ptr = nullptr;
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qpair<T>::value>::type from_ini(
        const inipp::Ini<char>::Section &section, T &pair)
    {
        auto it = section.find("first");
        if (it != section.end())
            string_to_val(it->second, pair.first);

        it = section.find("second");
        if (it != section.end())
            string_to_val(it->second, pair.second);
    }
#endif

    template<typename T>
    static
        typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value>::type
        from_ini(const inipp::Ini<char>::Section &section, T &map)
    {
        map.clear();
        for (const auto &pair : section) {
            typename T::key_type key;
            string_to_val(pair.first, key);

            typename T::mapped_type val;
            string_to_val(pair.second, val);

#ifdef O_SERIALIZE_USE_QT
            if constexpr (Traits::is_qt_map<T>::value)
                map.insert(key, val);
            else
#endif
                map[key] = val;
        }
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value>::type from_ini(
        const inipp::Ini<char>::Section &section, T &ptr)
    {
        auto it = section.find("null");
        if (it != section.end() && (it->second == "true" || it->second == "1")) {
            ptr.reset();
            return;
        }
        using ElementType = typename T::element_type;
        ptr = std::make_shared<ElementType>();
        from_ini(section, *ptr);
    }

    template<typename K, typename V>
    static void from_ini(const inipp::Ini<char>::Section &section, std::pair<K, V> &pair)
    {
        auto it = section.find("first");
        if (it != section.end())
            string_to_val(it->second, pair.first);

        it = section.find("second");
        if (it != section.end())
            string_to_val(it->second, pair.second);
    }

    template<typename Tuple, size_t... Is>
    static void tuple_from_ini_helper(const inipp::Ini<char>::Section &section,
                                      Tuple                           &t,
                                      std::index_sequence<Is...>)
    {
        (
            [&]() {
                auto it = section.find("item" + std::to_string(Is));
                if (it != section.end()) {
                    string_to_val(it->second, std::get<Is>(t));
                }
            }(),
            ...);
    }

    template<typename... Args>
    static typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value>::type from_ini(
        const inipp::Ini<char>::Section &section, std::tuple<Args...> &t)
    {
        tuple_from_ini_helper(section, t, std::index_sequence_for<Args...>{});
    }
};

} // namespace OSerialize

#endif // O_SERIALIZE_INI_H
