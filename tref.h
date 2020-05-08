/* TRef: A tiny compile time reflection system.
Author: soniced@sina.com

Refs:
https://woboq.com/blog/verdigris-implementation-tricks.html
https://www.codeproject.com/Articles/1002895/Clean-Reflective-Enums-Cplusplus-Enum-to-String-wi

*/

#pragma once
#include <array>
#include <string_view>
#include <type_traits>

#define TREF

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// Common

#define _TrefReturn(T) \
  ->decltype(T) { return T; }

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
};

template <class C>
struct member_object;

template <class T, class C>
struct member_object<T C::*> {
  using member_t = T;
  using object_t = C;
};

template <class T>
using remove_object_t = typename member_object<T>::member_t;

template <class T>
using object_t = typename member_object<T>::object_t;

//////////////////////////////////////////////////////////////////////////
/// macro version of map

#define _TrefMap(macro, arg, ...)                                    \
  _TrefIdentify(_TrefApply(_TrefChooseMap, _TrefCount(__VA_ARGS__))( \
      macro, arg, __VA_ARGS__))

#define _TrefChooseMap(N) _TrefMap##N
#define _TrefApply(macro, ...) _TrefIdentify(macro(__VA_ARGS__))
#define _TrefIdentify(x) x

#define _TrefMap1(m, a, x) m(a, x)
#define _TrefMap2(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap1(m, a, __VA_ARGS__))
#define _TrefMap3(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap2(m, a, __VA_ARGS__))
#define _TrefMap4(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap3(m, a, __VA_ARGS__))
#define _TrefMap5(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap4(m, a, __VA_ARGS__))
#define _TrefMap6(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap5(m, a, __VA_ARGS__))
#define _TrefMap7(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap6(m, a, __VA_ARGS__))
#define _TrefMap8(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap7(m, a, __VA_ARGS__))
#define _TrefMap9(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap8(m, a, __VA_ARGS__))
#define _TrefMap10(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap9(m, a, __VA_ARGS__))
#define _TrefMap11(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap10(m, a, __VA_ARGS__))
#define _TrefMap12(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap11(m, a, __VA_ARGS__))
#define _TrefMap13(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap12(m, a, __VA_ARGS__))
#define _TrefMap14(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap13(m, a, __VA_ARGS__))
#define _TrefMap15(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap14(m, a, __VA_ARGS__))
#define _TrefMap16(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap15(m, a, __VA_ARGS__))
#define _TrefMap17(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap16(m, a, __VA_ARGS__))
#define _TrefMap18(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap17(m, a, __VA_ARGS__))
#define _TrefMap19(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap18(m, a, __VA_ARGS__))
#define _TrefMap20(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap19(m, a, __VA_ARGS__))

#define _TrefEvaluateCount(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                           _13, _14, _15, _16, _17, _18, _19, _20, N, ...)    \
  N

#define _TrefCount(...)                                                     \
  _TrefIdentify(_TrefEvaluateCount(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, \
                                   13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

//////////////////////////////////////////////////////////////////////////

template <int N = 255>
struct Id : Id<N - 1> {
  enum { value = N };
};

template <>
struct Id<0> {
  enum { value = 0 };
};

template <typename C, typename T>
tuple<Id<0>> _tref_elem_info(C**, T, Id<0> id);

#define _TrefElemCnt(C, T)                                      \
  std::tuple_element_t<0, decltype(_tref_elem_info((C**)0, T{}, \
                                                   tref::imp::Id<>{}))>::value

#define _TrefPush(C, T, ...)                            \
  friend constexpr auto _tref_elem_info(                \
      C**, T, tref::imp::Id<_TrefElemCnt(C, T) + 1> id) \
      _TrefReturn(std::tuple(id, __VA_ARGS__))

template <class C, class T, class F, size_t... Is>
constexpr bool fold(index_sequence<Is...>, F&& f) {
  return (f(_tref_elem_info((C**)0, T{}, Id<Is>{})) && ...);
}

template <typename C, typename T, typename F>
constexpr bool each(F f) {
  constexpr auto cnt = _TrefElemCnt(C, T);
  if constexpr (cnt > 0) {
    return fold<C, T>(tail(make_index_sequence<cnt + 1>{}), f);
  } else
    return true;
};

//////////////////////////////////////////////////////////////////////////

struct DummyBase;
struct MemberTag {};
struct MemberTypeTag {};
struct SubclassTag {};

template <typename T, typename Base>
struct ClassInfo {
  using class_t = T;

  string_view name;
  size_t size;
  Base* base;

  constexpr ClassInfo(string_view n, size_t sz, Base* b)
      : name{n}, size{sz}, base{b} {}

  template <typename F>
  constexpr bool each_member(F&& f) const {
    return each<class_t, MemberTag>(
        [&](auto& info) { return f(get<1>(info)); });
  }

  // iterate members recursively.
  // @param F: [](MemberInfo info, int level)
  template <typename F>
  constexpr bool each_member_r(F&& f) const {
    // forward to the free function to delay the template evaluation.
    return imp::each_member_r<class_t>(forward<F>(f));
  }

  template <typename F>
  constexpr bool each_subclass(F&& f) const {
    return each<class_t, SubclassTag>([&](auto info) {
      using S = remove_pointer_t<tuple_element_t<1, decltype(info)>>;
      return f(class_info_v<S>);
    });
  }

  // iterate subclasses recursively.
  // @param F: [](ClassInfo info, int level)
  template <typename F>
  constexpr bool each_subclass_r(F&& f) const {
    // forward to the free function to delay the template evaluation.
    return imp::each_subclass_r<class_t>(forward<F>(f));
  }

  template <typename F>
  constexpr bool each_member_type(F&& f) const {
    return each<class_t, MemberTypeTag>(
        [&](auto info) { return f(get<1>(info)); });
  }
};

template <typename T, typename Meta>
struct MemberInfo {
  string_view name;
  T addr;
  Meta meta;

  constexpr MemberInfo(string_view n, T a, Meta m)
      : name{n}, addr{a}, meta{m} {}
};

void _tref_class_info(void*);

#define _TrefClassMeta(T, Base)                                           \
  friend constexpr auto _tref_class_info(T**) {                           \
    return tref::imp::ClassInfo<T, Base>{std::string_view(#T), sizeof(T), \
                                         (Base*)0};                       \
  }

template <typename T>
constexpr auto is_reflected_v =
    !std::is_same_v<decltype(_tref_class_info((T**)0)), void>;

template <typename T>
constexpr auto class_info_v = _tref_class_info((T**)0);

template <typename T>
constexpr auto has_base_v =
    is_reflected_v<T> &&
    !std::is_same_v<decltype(class_info_v<T>.base), DummyBase*>;

template <typename T>
using base_class_t = remove_pointer_t<decltype(class_info_v<T>.base)>;

template <class C, typename F>
constexpr bool each_member_r(F&& f, int level = 0) {
  auto next =
      each<C, MemberTag>([&](auto& info) { return f(get<1>(info), level); });
  if (next)
    if constexpr (has_base_v<C>)
      return each_member_r<base_class_t<C>>(f, level + 1);
  return true;
}

template <class C, typename F>
constexpr bool each_subclass_r(F&& f, int level = 0) {
  return each<C, SubclassTag>([&](auto info) {
    using S = remove_pointer_t<tuple_element_t<1, decltype(info)>>;
    return f(class_info_v<S>, level) && each_subclass_r<S>(f, level + 1);
  });
}

//////////////////////////////////////////////////////////////////////////

template <typename T, class = void_t<>>
struct get_parent {
  using type = DummyBase;
};

template <typename T>
struct get_parent<T, void_t<decltype((typename T::__parent_t*)0)>> {
  using type = typename T::__parent_t;
};

template <typename T>
using base_t = typename get_parent<T>::type;

#define _TrefRegisterSubclass(base) \
  _TrefPush(base, tref::imp::SubclassTag, (self_t*)0)

#define _TrefTypeCommon(T, Base) \
 private:                        \
  using self_t = T;              \
                                 \
 public:                         \
  using __parent_t = T;          \
  _TrefClassMeta(T, Base);

#define _TrefMember(Tag, F, val, ...) \
  _TrefPush(self_t, Tag,              \
            tref::imp::MemberInfo{std::string_view(#F), val, __VA_ARGS__})

// Reflect the type and register it into a hierarchy tree.
// NOTE: the entire tree should be in same namespace.
#define TrefRootType(T) _TrefTypeCommon(T, tref::imp::DummyBase);
#define TrefSubType(T) \
  TrefType(T);         \
  _TrefRegisterSubclass(__base_t);

// Just reflect the type.
#define TrefType(T)                      \
 private:                                \
  using __base_t = tref::imp::base_t<T>; \
  _TrefTypeCommon(T, __base_t);

// Reflect the external type and register it into a hierarchy tree.
// NOTE: the entire tree should be in same namespace.
#define TrefExternalRootType(T) _TrefTypeCommon(T, tref::imp::DummyBase)
#define TrefExternalSubType(T, Base) \
  _TrefTypeCommon(T, Base);          \
  _TrefRegisterSubclass(Base);

// Just reflect the external type.
#define TrefExternalType(T, Base) _TrefTypeCommon(T, Base)

// reflect member variable & function
#define TrefMember(F) TrefMemberWithMeta(F, 0)
#define TrefMemberWithMeta(F, ...) \
  _TrefMember(tref::imp::MemberTag, F, &self_t::F, __VA_ARGS__)

// reflect member type
#define TrefMemberType(F) TrefMemberTypeWithMeta(F, 0)
#define TrefMemberTypeWithMeta(F, ...) \
  _TrefMember(tref::imp::MemberTypeTag, F, (F*)0, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
// enum reflection
//////////////////////////////////////////////////////////////////////////

struct EnumValue {
  template <typename T>
  constexpr EnumValue(T v) : value((size_t)v) {}
  template <typename U>
  constexpr EnumValue operator=(U) {
    return *this;
  }
  size_t value;
};

template <typename T, size_t N>
struct EnumInfo {
  using enum_t = T;

  string_view name;
  size_t size;
  array<pair<string_view, EnumValue>, N> items;

  // @param f: [](string_view name, enum_t val)
  template <typename F>
  constexpr auto each_item(F&& f) const {
    for (auto& e : items) {
      f(get<0>(e), (enum_t)get<1>(e).value);
    }
  }
};

void* _tref_enum_info(void*);

template <typename T, typename = enable_if_t<is_enum_v<T>, bool>>
constexpr auto enum_info_v = _tref_enum_info((T**)0);

#define TrefEnumGlobal(T, ...)  \
  enum class T { __VA_ARGS__ }; \
  TrefEnumImp(T, __VA_ARGS__)

#define TrefEnum(T, ...)        \
  enum class T { __VA_ARGS__ }; \
  friend TrefEnumImp(T, __VA_ARGS__)

// reflect enum items
#define TrefEnumImp(T, ...)                                 \
  constexpr auto _tref_enum_info(T**) {                     \
    return tref::imp::EnumInfo<T, _TrefCount(__VA_ARGS__)>{ \
        std::string_view(#T),                               \
        sizeof(T),                                          \
        {_TrefEnumStringize(T, __VA_ARGS__)}};              \
  }

#define _TrefEnumStringizeSingle(P, E) \
  std::pair{tref::imp::enum_trim_name(#E), (tref::imp::EnumValue)P::E},

#define _TrefEnumStringize(P, ...) \
  _TrefIdentify(_TrefMap(_TrefEnumStringizeSingle, P, __VA_ARGS__))

constexpr string_view enum_trim_name(string_view s) {
  auto p = s.find_first_of(' ');
  if (p == string_view::npos)
    p = s.find_first_of('=');
  return s.substr(0, p);
}

template <typename T>
constexpr auto enum_to_string(T v) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info_v<T>.items) {
    if (e.second.value == (int)v) {
      return e.first;
    }
  }
  return string_view{};
}

template <typename T>
constexpr auto string_to_enum(string_view s, T defVal) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info_v<T>.items) {
    if (s == e.first) {
      return (T)e.second.value;
    }
  }
  return defVal;
}

template <typename T, typename Storage = unsigned int>
struct Flags {
  static_assert(std::is_enum_v<T>);
  Storage value = 0;

  constexpr Flags() {}
  constexpr void clear() { value = 0; }
  constexpr bool hasFlag(T e) {
    assert(e < sizeof(value) * 8);
    return (value & (1 << static_cast<Storage>(e))) != 0;
  }
  constexpr void setFlag(T e) {
    assert(e < sizeof(value) * 8);
    value |= (1 << static_cast<Storage>(e));
  }
  constexpr void clearFlag(T e) {
    assert(e < sizeof(value) * 8);
    value &= ~(1 << static_cast<Storage>(e));
  }
};

}  // namespace imp

using imp::base_class_t;
using imp::class_info_v;
using imp::has_base_v;
using imp::is_reflected_v;
using imp::object_t;
using imp::remove_object_t;

using imp::enum_info_v;
using imp::enum_to_string;
using imp::Flags;
using imp::string_to_enum;

}  // namespace tref
