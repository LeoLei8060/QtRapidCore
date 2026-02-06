#ifndef O_SERIALIZE_H
#define O_SERIALIZE_H

#include <tuple>
#include <functional>
#include <type_traits>

namespace OSerialize {

namespace Meta {

    template <typename T>
    struct Reflector {
        static constexpr bool is_defined = false;
    };

    template <typename T>
    struct has_reflection {
        static constexpr bool value = Reflector<T>::is_defined;
    };

    template <typename T, typename Visitor>
    typename std::enable_if<has_reflection<T>::value>::type
    visit_members(T& obj, Visitor&& visitor) {
        Reflector<T>::visit(obj, std::forward<Visitor>(visitor));
    }

    template <typename T, typename Visitor>
    typename std::enable_if<has_reflection<T>::value>::type
    visit_members(const T& obj, Visitor&& visitor) {
        Reflector<T>::visit(obj, std::forward<Visitor>(visitor));
    }

} // namespace Meta

} // namespace OSerialize

#define EXPAND(x) x
#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, NAME, ...) NAME

#define FE_1(action, x) action(x)
#define FE_2(action, x, ...) action(x) FE_1(action, __VA_ARGS__)
#define FE_3(action, x, ...) action(x) FE_2(action, __VA_ARGS__)
#define FE_4(action, x, ...) action(x) FE_3(action, __VA_ARGS__)
#define FE_5(action, x, ...) action(x) FE_4(action, __VA_ARGS__)
#define FE_6(action, x, ...) action(x) FE_5(action, __VA_ARGS__)
#define FE_7(action, x, ...) action(x) FE_6(action, __VA_ARGS__)
#define FE_8(action, x, ...) action(x) FE_7(action, __VA_ARGS__)
#define FE_9(action, x, ...) action(x) FE_8(action, __VA_ARGS__)
#define FE_10(action, x, ...) action(x) FE_9(action, __VA_ARGS__)
#define FE_11(action, x, ...) action(x) FE_10(action, __VA_ARGS__)
#define FE_12(action, x, ...) action(x) FE_11(action, __VA_ARGS__)
#define FE_13(action, x, ...) action(x) FE_12(action, __VA_ARGS__)
#define FE_14(action, x, ...) action(x) FE_13(action, __VA_ARGS__)
#define FE_15(action, x, ...) action(x) FE_14(action, __VA_ARGS__)
#define FE_16(action, x, ...) action(x) FE_15(action, __VA_ARGS__)
#define FE_17(action, x, ...) action(x) FE_16(action, __VA_ARGS__)
#define FE_18(action, x, ...) action(x) FE_17(action, __VA_ARGS__)
#define FE_19(action, x, ...) action(x) FE_18(action, __VA_ARGS__)
#define FE_20(action, x, ...) action(x) FE_19(action, __VA_ARGS__)
#define FE_21(action, x, ...) action(x) FE_20(action, __VA_ARGS__)
#define FE_22(action, x, ...) action(x) FE_21(action, __VA_ARGS__)
#define FE_23(action, x, ...) action(x) FE_22(action, __VA_ARGS__)
#define FE_24(action, x, ...) action(x) FE_23(action, __VA_ARGS__)
#define FE_25(action, x, ...) action(x) FE_24(action, __VA_ARGS__)

#define FOR_EACH(action, ...) \
    GET_MACRO(__VA_ARGS__, FE_25, FE_24, FE_23, FE_22, FE_21, FE_20, FE_19, FE_18, FE_17, FE_16, FE_15, FE_14, FE_13, FE_12, FE_11, FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)(action, __VA_ARGS__)

// The action will be: visitor(#member, obj.member);
#define VISIT_MEMBER(member) visitor(#member, obj.member);

#define O_SERIALIZE_STRUCT(Type, ...) \
namespace OSerialize { namespace Meta { \
    template <> \
    struct Reflector<Type> { \
        static constexpr bool is_defined = true; \
        \
        template <typename Visitor> \
        static void visit(Type& obj, Visitor&& visitor) { \
            FOR_EACH(VISIT_MEMBER, __VA_ARGS__) \
        } \
        \
        template <typename Visitor> \
        static void visit(const Type& obj, Visitor&& visitor) { \
            FOR_EACH(VISIT_MEMBER, __VA_ARGS__) \
        } \
    }; \
}}

#endif // O_SERIALIZE_H
