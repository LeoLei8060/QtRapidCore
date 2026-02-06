#ifndef TOUIUPDATEMSG_H
#define TOUIUPDATEMSG_H

/**
  * @brief: UI渲染数据结构，由解析线程发送给UI线程的
  * @author: Leo
  */

#include "imessages.h"
#include <QObject>

// Example:
// class XXXToUiMsg : public IUIUpdateMsg
// {
//     Q_OBJECT
// public:
//     explicit XXXToUiMsg(QObject *parent = nullptr)
//         : IUIUpdateMsg(parent)
//     {
//         setMsgType(UiUpdateMsgType::kXXXToUiMsg);
//     }

//     // member data
// };
// using XXXToUiMsgPtr = QSharedPointer<XXXToUiMsg>;

#endif // TOUIUPDATEMSG_H
