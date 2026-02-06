#ifndef O_SERIALIZE_XML_H
#define O_SERIALIZE_XML_H

#include "base.h"
#include "o_serialize.h"
#include "tinyxml/tinyxml2.h"
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

class XML
{
public:
    template<typename T>
    static std::string obj_to_string(const T &obj, const std::string &rootName = "root")
    {
        tinyxml2::XMLDocument     doc;
        tinyxml2::XMLDeclaration *decl = doc.NewDeclaration();
        doc.InsertFirstChild(decl);

        tinyxml2::XMLElement *root = doc.NewElement(rootName.c_str());
        doc.InsertEndChild(root);

        to_xml(obj, root, doc);

        tinyxml2::XMLPrinter printer;
        doc.Print(&printer);
        return printer.CStr();
    }

    template<typename T>
    static T string_to_obj(const std::string &xml, const std::string &rootName = "root")
    {
        tinyxml2::XMLDocument doc;
        doc.Parse(xml.c_str());

        T obj;
        if (doc.Error()) {
            std::cerr << "XML Parse Error: " << doc.ErrorStr() << std::endl;
            return obj;
        }

        tinyxml2::XMLElement *root = doc.FirstChildElement(rootName.c_str());
        if (!root) {
            std::cerr << "XML Parse Error: Root element '" << rootName << "' not found."
                      << std::endl;
            return obj;
        }

        from_xml(root, obj);
        return obj;
    }

    template<typename T>
    static bool obj_to_file(const T           &obj,
                            const std::string &filepath,
                            const std::string &rootName = "root")
    {
        tinyxml2::XMLDocument     doc;
        tinyxml2::XMLDeclaration *decl = doc.NewDeclaration();
        doc.InsertFirstChild(decl);

        tinyxml2::XMLElement *root = doc.NewElement(rootName.c_str());
        doc.InsertEndChild(root);

        to_xml(obj, root, doc);

        return doc.SaveFile(filepath.c_str()) == tinyxml2::XML_SUCCESS;
    }

    template<typename T>
    static T file_to_obj(const std::string &filepath, const std::string &rootName = "root")
    {
        T                     obj;
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filepath.c_str()) != tinyxml2::XML_SUCCESS) {
            std::cerr << "Cannot open/parse file: " << filepath << " Error: " << doc.ErrorStr()
                      << std::endl;
            return obj;
        }

        tinyxml2::XMLElement *root = doc.FirstChildElement(rootName.c_str());
        if (!root) {
            std::cerr << "XML Parse Error: Root element '" << rootName << "' not found."
                      << std::endl;
            return obj;
        }

        from_xml(root, obj);
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
    static typename std::enable_if<Meta::has_reflection<T>::value, void>::type to_xml(
        const T &obj, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        Meta::visit_members(obj, [&](const char *name, const auto &member) {
            tinyxml2::XMLElement *child = doc.NewElement(name);
            element->InsertEndChild(child);
            to_xml(member, child, doc);
        });
    }

    template<typename T>
    static typename std::enable_if<std::is_arithmetic<T>::value, void>::type to_xml(
        const T &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val);
    }

    template<typename T>
    static typename std::enable_if<std::is_enum<T>::value, void>::type to_xml(
        T val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText((int) val);
    }

    static void to_xml(const std::string     &val,
                       tinyxml2::XMLElement  *element,
                       tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.c_str());
    }

    static void to_xml(const char *val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val);
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value, void>::type to_xml(
        const T &ptr, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        if (ptr) {
            to_xml(*ptr, element, doc);
        } else {
            element->SetAttribute("null", true);
        }
    }

    template<typename K, typename V>
    static void to_xml(const std::pair<K, V> &pair,
                       tinyxml2::XMLElement  *element,
                       tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *first = doc.NewElement("first");
        element->InsertEndChild(first);
        to_xml(pair.first, first, doc);

        tinyxml2::XMLElement *second = doc.NewElement("second");
        element->InsertEndChild(second);
        to_xml(pair.second, second, doc);
    }

    template<typename... Args>
    static void to_xml(const std::variant<Args...> &v,
                       tinyxml2::XMLElement        *element,
                       tinyxml2::XMLDocument       &doc)
    {
        std::visit([&](const auto &val) { to_xml(val, element, doc); }, v);
    }

    template<typename Tuple, size_t... Is>
    static void tuple_to_xml_helper(const Tuple           &t,
                                    tinyxml2::XMLElement  *element,
                                    tinyxml2::XMLDocument &doc,
                                    std::index_sequence<Is...>)
    {
        (
            [&]() {
                tinyxml2::XMLElement *item = doc.NewElement("item");
                element->InsertEndChild(item);
                to_xml(std::get<Is>(t), item, doc);
            }(),
            ...);
    }

    template<typename... Args>
    static typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value, void>::type to_xml(
        const std::tuple<Args...> &t, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tuple_to_xml_helper(t, element, doc, std::index_sequence_for<Args...>{});
    }

#ifdef O_SERIALIZE_USE_QT
    static void to_xml(const QString &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.toUtf8().constData());
    }

    static void to_xml(const QDate &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.toString(Qt::ISODate).toUtf8().constData());
    }
    static void to_xml(const QTime &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.toString(Qt::ISODate).toUtf8().constData());
    }
    static void to_xml(const QDateTime       &val,
                       tinyxml2::XMLElement  *element,
                       tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.toString(Qt::ISODate).toUtf8().constData());
    }

    static void to_xml(const QPoint &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *x = doc.NewElement("x");
        x->SetText(val.x());
        element->InsertEndChild(x);
        tinyxml2::XMLElement *y = doc.NewElement("y");
        y->SetText(val.y());
        element->InsertEndChild(y);
    }
    static void to_xml(const QPointF &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *x = doc.NewElement("x");
        x->SetText(val.x());
        element->InsertEndChild(x);
        tinyxml2::XMLElement *y = doc.NewElement("y");
        y->SetText(val.y());
        element->InsertEndChild(y);
    }
    static void to_xml(const QSize &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *w = doc.NewElement("width");
        w->SetText(val.width());
        element->InsertEndChild(w);
        tinyxml2::XMLElement *h = doc.NewElement("height");
        h->SetText(val.height());
        element->InsertEndChild(h);
    }
    static void to_xml(const QSizeF &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *w = doc.NewElement("width");
        w->SetText(val.width());
        element->InsertEndChild(w);
        tinyxml2::XMLElement *h = doc.NewElement("height");
        h->SetText(val.height());
        element->InsertEndChild(h);
    }
    static void to_xml(const QRect &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *x = doc.NewElement("x");
        x->SetText(val.x());
        element->InsertEndChild(x);
        tinyxml2::XMLElement *y = doc.NewElement("y");
        y->SetText(val.y());
        element->InsertEndChild(y);
        tinyxml2::XMLElement *w = doc.NewElement("width");
        w->SetText(val.width());
        element->InsertEndChild(w);
        tinyxml2::XMLElement *h = doc.NewElement("height");
        h->SetText(val.height());
        element->InsertEndChild(h);
    }
    static void to_xml(const QRectF &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *x = doc.NewElement("x");
        x->SetText(val.x());
        element->InsertEndChild(x);
        tinyxml2::XMLElement *y = doc.NewElement("y");
        y->SetText(val.y());
        element->InsertEndChild(y);
        tinyxml2::XMLElement *w = doc.NewElement("width");
        w->SetText(val.width());
        element->InsertEndChild(w);
        tinyxml2::XMLElement *h = doc.NewElement("height");
        h->SetText(val.height());
        element->InsertEndChild(h);
    }

    static void to_xml(const QColor &val, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.name().toUtf8().constData());
    }

    static void to_xml(const QByteArray      &val,
                       tinyxml2::XMLElement  *element,
                       tinyxml2::XMLDocument &doc)
    {
        element->SetText(val.toStdString().c_str());
    }

    static void to_xml(const QVariant        &val,
                       tinyxml2::XMLElement  *element,
                       tinyxml2::XMLDocument &doc)
    {
        switch (val.type()) {
        case QVariant::Int:
            to_xml(val.toInt(), element, doc);
            break;
        case QVariant::UInt:
            to_xml(val.toUInt(), element, doc);
            break;
        case QVariant::LongLong:
            to_xml(val.toLongLong(), element, doc);
            break;
        case QVariant::ULongLong:
            to_xml(val.toULongLong(), element, doc);
            break;
        case QVariant::Double:
            to_xml(val.toDouble(), element, doc);
            break;
        case QVariant::Bool:
            to_xml(val.toBool(), element, doc);
            break;
        case QVariant::String:
            to_xml(val.toString(), element, doc);
            break;
        case QVariant::StringList:
            to_xml(val.toStringList(), element, doc);
            break;
        case QVariant::List:
            to_xml(val.toList(), element, doc);
            break;
        case QVariant::Map:
            to_xml(val.toMap(), element, doc);
            break;
        case QVariant::Hash:
            to_xml(val.toHash(), element, doc);
            break;
        default:
            if (val.canConvert<QString>())
                to_xml(val.toString(), element, doc);
            break;
        }
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value, void>::type to_xml(
        const T &ptr, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        if (ptr) {
            to_xml(*ptr, element, doc);
        } else {
            element->SetAttribute("null", true);
        }
    }
#endif

    template<typename T>
    static typename std::enable_if<Traits::is_stl_container<T>::value
                                       || Traits::is_qt_container<T>::value,
                                   void>::type
    to_xml(const T &container, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        for (const auto &item : container) {
            tinyxml2::XMLElement *child = doc.NewElement("item");
            element->InsertEndChild(child);
            to_xml(item, child, doc);
        }
    }

    template<typename T>
    static
        typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value, void>::type
        to_xml(const T &map, tinyxml2::XMLElement *element, tinyxml2::XMLDocument &doc)
    {
        // Map 元素默认使用 <item key="...">...</item> 结构
        for (auto it = map.begin(); it != map.end(); ++it) {
            tinyxml2::XMLElement *child = doc.NewElement("item");

            std::string keyStr;
            if constexpr (std::is_same_v<typename T::key_type, std::string>) {
                keyStr = it->first;
            }
#ifdef O_SERIALIZE_USE_QT
            else if constexpr (std::is_same_v<typename T::key_type, QString>) {
                keyStr = it.key().toStdString();
            }
#endif
            else {
                // 对于非字符串 Key，转换为字符串
#ifdef O_SERIALIZE_USE_QT
                if constexpr (Traits::is_qt_map<T>::value)
                    keyStr = std::to_string(it.key()); // 简单处理，假设是数值
                else
#endif
                    keyStr = std::to_string(it->first);
            }

            child->SetAttribute("key", keyStr.c_str());
            element->InsertEndChild(child);

#ifdef O_SERIALIZE_USE_QT
            if constexpr (Traits::is_qt_map<T>::value)
                to_xml(it.value(), child, doc);
            else
#endif
                to_xml(it->second, child, doc);
        }
    }

    template<typename T>
    static typename std::enable_if<Meta::has_reflection<T>::value>::type from_xml(
        tinyxml2::XMLElement *element, T &obj)
    {
        if (!element)
            return;
        Meta::visit_members(obj, [&](const char *name, auto &member) {
            tinyxml2::XMLElement *child = element->FirstChildElement(name);
            if (child) {
                from_xml(child, member);
            }
        });
    }

    static void from_xml(tinyxml2::XMLElement *element, int &val)
    {
        if (element && element->GetText())
            val = std::stoi(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, unsigned int &val)
    {
        if (element && element->GetText())
            val = std::stoul(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, long &val)
    {
        if (element && element->GetText())
            val = std::stol(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, unsigned long &val)
    {
        if (element && element->GetText())
            val = std::stoul(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, long long &val)
    {
        if (element && element->GetText())
            val = std::stoll(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, unsigned long long &val)
    {
        if (element && element->GetText())
            val = std::stoull(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, double &val)
    {
        if (element && element->GetText())
            val = std::stod(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, float &val)
    {
        if (element && element->GetText())
            val = std::stof(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, bool &val)
    {
        if (element && element->GetText()) {
            std::string text = element->GetText();
            val = (text == "true" || text == "1");
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, signed char &val)
    {
        if (element && element->GetText())
            val = (signed char) std::stoi(element->GetText());
    }
    static void from_xml(tinyxml2::XMLElement *element, unsigned char &val)
    {
        if (element && element->GetText())
            val = (unsigned char) std::stoul(element->GetText());
    }
    template<typename T>
    static typename std::enable_if<std::is_enum<T>::value>::type from_xml(
        tinyxml2::XMLElement *element, T &val)
    {
        if (element && element->GetText())
            val = static_cast<T>(std::stoi(element->GetText()));
    }

    static void from_xml(tinyxml2::XMLElement *element, std::string &val)
    {
        if (element && element->GetText())
            val = element->GetText();
    }

    template<typename T>
    static typename std::enable_if<Traits::is_smart_ptr<T>::value>::type from_xml(
        tinyxml2::XMLElement *element, T &ptr)
    {
        if (!element) {
            ptr.reset();
            return;
        }
        bool isNull = false;
        if (element->QueryBoolAttribute("null", &isNull) == tinyxml2::XML_SUCCESS && isNull) {
            ptr.reset();
            return;
        }

        using ElementType = typename T::element_type;
        ptr = std::make_shared<ElementType>();
        from_xml(element, *ptr);
    }

    template<typename K, typename V>
    static void from_xml(tinyxml2::XMLElement *element, std::pair<K, V> &pair)
    {
        if (!element)
            return;
        tinyxml2::XMLElement *first = element->FirstChildElement("first");
        if (first)
            from_xml(first, pair.first);

        tinyxml2::XMLElement *second = element->FirstChildElement("second");
        if (second)
            from_xml(second, pair.second);
    }

    template<typename Tuple, size_t... Is>
    static void tuple_from_xml_helper(tinyxml2::XMLElement *element,
                                      Tuple                &t,
                                      std::index_sequence<Is...>)
    {
        if (!element)
            return;
        tinyxml2::XMLElement *child = element->FirstChildElement("item");
        (
            [&]() {
                if (child) {
                    from_xml(child, std::get<Is>(t));
                    child = child->NextSiblingElement("item");
                }
            }(),
            ...);
    }

    template<typename... Args>
    static typename std::enable_if<Traits::is_tuple<std::tuple<Args...>>::value>::type from_xml(
        tinyxml2::XMLElement *element, std::tuple<Args...> &t)
    {
        tuple_from_xml_helper(element, t, std::index_sequence_for<Args...>{});
    }

#ifdef O_SERIALIZE_USE_QT
    static void from_xml(tinyxml2::XMLElement *element, QString &val)
    {
        if (element && element->GetText())
            val = QString::fromUtf8(element->GetText());
    }

    static void from_xml(tinyxml2::XMLElement *element, QDate &val)
    {
        if (element && element->GetText())
            val = QDate::fromString(QString::fromUtf8(element->GetText()), Qt::ISODate);
    }
    static void from_xml(tinyxml2::XMLElement *element, QTime &val)
    {
        if (element && element->GetText())
            val = QTime::fromString(QString::fromUtf8(element->GetText()), Qt::ISODate);
    }
    static void from_xml(tinyxml2::XMLElement *element, QDateTime &val)
    {
        if (element && element->GetText())
            val = QDateTime::fromString(QString::fromUtf8(element->GetText()), Qt::ISODate);
    }

    static void from_xml(tinyxml2::XMLElement *element, QPoint &val)
    {
        if (element) {
            tinyxml2::XMLElement *c = element->FirstChildElement("x");
            if (c)
                from_xml(c, val.rx());
            c = element->FirstChildElement("y");
            if (c)
                from_xml(c, val.ry());
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, QPointF &val)
    {
        if (element) {
            tinyxml2::XMLElement *c = element->FirstChildElement("x");
            if (c)
                from_xml(c, val.rx());
            c = element->FirstChildElement("y");
            if (c)
                from_xml(c, val.ry());
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, QSize &val)
    {
        if (element) {
            tinyxml2::XMLElement *c = element->FirstChildElement("width");
            if (c)
                from_xml(c, val.rwidth());
            c = element->FirstChildElement("height");
            if (c)
                from_xml(c, val.rheight());
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, QSizeF &val)
    {
        if (element) {
            tinyxml2::XMLElement *c = element->FirstChildElement("width");
            if (c)
                from_xml(c, val.rwidth());
            c = element->FirstChildElement("height");
            if (c)
                from_xml(c, val.rheight());
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, QRect &val)
    {
        if (element) {
            int                   x = 0, y = 0, w = 0, h = 0;
            tinyxml2::XMLElement *c = element->FirstChildElement("x");
            if (c)
                from_xml(c, x);
            c = element->FirstChildElement("y");
            if (c)
                from_xml(c, y);
            c = element->FirstChildElement("width");
            if (c)
                from_xml(c, w);
            c = element->FirstChildElement("height");
            if (c)
                from_xml(c, h);
            val.setRect(x, y, w, h);
        }
    }
    static void from_xml(tinyxml2::XMLElement *element, QRectF &val)
    {
        if (element) {
            double                x = 0, y = 0, w = 0, h = 0;
            tinyxml2::XMLElement *c = element->FirstChildElement("x");
            if (c)
                from_xml(c, x);
            c = element->FirstChildElement("y");
            if (c)
                from_xml(c, y);
            c = element->FirstChildElement("width");
            if (c)
                from_xml(c, w);
            c = element->FirstChildElement("height");
            if (c)
                from_xml(c, h);
            val.setRect(x, y, w, h);
        }
    }

    static void from_xml(tinyxml2::XMLElement *element, QColor &val)
    {
        if (element && element->GetText())
            val.setNamedColor(QString::fromUtf8(element->GetText()));
    }

    static void from_xml(tinyxml2::XMLElement *element, QByteArray &val)
    {
        if (element && element->GetText())
            val = QByteArray(element->GetText());
    }

    static void from_xml(tinyxml2::XMLElement *element, QVariant &val)
    {
        // XML 反序列化 QVariant 比较困难，因为丢失了类型信息
        // 这里尝试基本推断，或者全部作为 String
        if (!element) {
            val = QVariant();
            return;
        }

        // 如果有子节点，可能是 List 或 Map
        if (element->FirstChildElement()) {
            // 简单假设，不支持复杂嵌套推断，或者需要额外的类型属性
            // 现阶段只能留空或作为 String
        }

        const char *text = element->GetText();
        if (text) {
            val = QString::fromUtf8(text);
        } else {
            val = QVariant();
        }
    }

    template<typename T>
    static typename std::enable_if<Traits::is_qt_smart_ptr<T>::value>::type from_xml(
        tinyxml2::XMLElement *element, T &ptr)
    {
        if (!element) {
            ptr.reset();
            return;
        }
        bool isNull = false;
        if (element->QueryBoolAttribute("null", &isNull) == tinyxml2::XML_SUCCESS && isNull) {
            ptr.reset();
            return;
        }
        create_qt_smart_ptr_xml(ptr, element);
    }

    template<typename T>
    static auto create_qt_smart_ptr_xml(QSharedPointer<T> &ptr, tinyxml2::XMLElement *element)
        -> void
    {
        ptr = QSharedPointer<T>::create();
        from_xml(element, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr_xml(QScopedPointer<T> &ptr, tinyxml2::XMLElement *element)
        -> void
    {
        ptr.reset(new T());
        from_xml(element, *ptr);
    }

    template<typename T>
    static auto create_qt_smart_ptr_xml(QPointer<T> &ptr, tinyxml2::XMLElement *) -> void
    {
        ptr = nullptr;
    }
#endif

    template<typename T>
    static typename std::enable_if<Traits::is_stl_container<T>::value
                                   || Traits::is_qt_container<T>::value>::type
    from_xml(tinyxml2::XMLElement *element, T &container)
    {
        if (!element)
            return;
        container.clear();
        tinyxml2::XMLElement *child = element->FirstChildElement("item");
        while (child) {
            typename T::value_type val;
            from_xml(child, val);
            add_item(container, val);
            child = child->NextSiblingElement("item");
        }
    }

    template<typename T>
    static
        typename std::enable_if<Traits::is_stl_map<T>::value || Traits::is_qt_map<T>::value>::type
        from_xml(tinyxml2::XMLElement *element, T &map)
    {
        if (!element)
            return;
        map.clear();
        tinyxml2::XMLElement *child = element->FirstChildElement("item");
        while (child) {
            const char *keyAttr = child->Attribute("key");
            if (keyAttr) {
                typename T::mapped_type val;
                from_xml(child, val);
                insert_map_item(map, keyAttr, val);
            }
            child = child->NextSiblingElement("item");
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

#endif // O_SERIALIZE_XML_H
