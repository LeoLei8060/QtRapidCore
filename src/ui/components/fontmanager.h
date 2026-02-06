#ifndef FONTMANAGER_H
#define FONTMANAGER_H

/**
  * @brief: Icon字体库的管理类
  * 提供加载字体库文件、字体样式管理
  * @author: Leo
  */

#include <QFont>
#include <QFontDatabase>
#include <QMap>

class FontManager
{
public:
    enum EFontType { kIcon = 0, kRegular, kSemiBold, kOther };

    static FontManager *instance();

    bool  addThirdpartyFont(const QString &path, EFontType type);
    QFont fontAt(EFontType type);
    bool  setFont_Bold(EFontType type, bool bold);

private:
    FontManager() = default;

    QMap<EFontType, QFont> m_fontsMap;
};

#endif // FONTMANAGER_H
