#include "fontmanager.h"

FontManager *FontManager::instance()
{
    static FontManager m_Instance;
    return &m_Instance;
}

bool FontManager::addThirdpartyFont(const QString &path, EFontType type)
{
    int fontID = QFontDatabase::addApplicationFont(path);
    if (fontID < 0)
        return false;

    QString fontName = QFontDatabase::applicationFontFamilies(fontID).at(0);
    QFont   font(fontName);
    m_fontsMap[type] = font;
    return true;
}

QFont FontManager::fontAt(EFontType type)
{
    if (m_fontsMap.find(type) != m_fontsMap.end())
        return m_fontsMap[type];
    return QFont();
}

bool FontManager::setFont_Bold(EFontType type, bool bold)
{
    if (m_fontsMap.find(type) == m_fontsMap.end())
        return false;
    m_fontsMap[type].setBold(bold);
    return true;
}
