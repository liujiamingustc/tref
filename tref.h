/* TRef: A tiny compile time reflection system.
Author: soniced@sina.com

Refs:
https://woboq.com/blog/verdigris-implementation-tricks.html
*/

#pragma once
#include <tuple>

#define TREF

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////

template <int N>
struct Id : Id<N - 1> {
  enum { value = N };
};

template <>
struct Id<0> {
  enum { value = 0 };
};

using MaxId = Id<255>;

template <typename C, typename T>
tuple<Id<0>> _tref_elem(C**, T, Id<0> id);

#define _TrefElemCnt(C, T)                                 \
  std::tuple_element_t<0, decltype(_tref_elem((C**)0, T{}, \
                                              tref::imp::MaxId{}))>::value

#define _TrefNextId(C, T) tref::imp::Id<_TrefElemCnt(C, T) + 1>

#define _TrefReturn(T) \
  ->decltype(T) { return T; }

#define _TrefPush(C, T, ...)                                     \
  friend constexpr auto _tref_elem(C**, T, _TrefNextId(C, T) id) \
      _TrefReturn(std::make_tuple(id, __VA_ARGS__))

template <class C, class T, class F, size_t... Is>
constexpr bool fold(index_sequence<Is...>, F&& f) {
  return (f(_tref_elem((C**)0, T{}, Id<Is>{})) && ...);
}

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
};

template <typename C, typename T, typename F>
constexpr bool each(F f) {
  constexpr auto cnt = _TrefElemCnt(C, T);
  if constexpr (cnt > 0) {
    return fold<C, T>(tail(make_index_sequence<cnt + 1>{}), f);
  } else
    return true;
};

//////////////////////////////////////////////////////////////////////////
/// base trait

void _tref_has_base(void*);

#define _TrefHasBase(T, Base) \
  friend constexpr Base* _tref_has_base(T**) { return nullptr; }

template <typename T>
constexpr auto has_base_v =
    !std::is_same_v<decltype(_tref_has_base((T**)0)), void>;

template <typename T>
using base_class_t = remove_pointer_t<decltype(_tref_has_base((T**)0))>;

//////////////////////////////////////////////////////////////////////////

false_type _tref_class_meta(void*);

template <typename T>
constexpr auto is_reflected_v =
    !std::is_same_v<decltype(_tref_class_meta((T**)0)), false_type>;

template <typename T>
constexpr auto class_meta_v = _tref_class_meta((T**)0);

#define _TrefClassMeta(T)                       \
  friend constexpr auto _tref_class_meta(T**) { \
    return std::make_tuple(#T, sizeof(T));      \
  }

//////////////////////////////////////////////////////////////////////////

struct MemberTag {};
struct SubclassTag {};

#define TrefTypeRoot(T) _TrefTypeCommon(T)
#define TrefType(T)       \
 private:                 \
  using base_t = _parent; \
  _TrefTypeCommon(T);     \
  _TrefBase(base_t);

#define TrefTypeExternalRoot(T) _TrefTypeCommon(T)
#define TrefTypeExternal(T, Base) \
  _TrefTypeCommon(T);             \
  _TrefBase(Base);

#define _TrefBase(base)       \
  _TrefHasBase(self_t, base); \
  _TrefPush(base, tref::imp::SubclassTag, (self_t*)0)

#define _TrefTypeCommon(T) \
 private:                  \
  using self_t = T;        \
                           \
 protected:                \
  using _parent = T;       \
                           \
 public:                   \
  _TrefClassMeta(T);

#define TrefMemberWithMeta(F, ...) _TrefMember(F, &self_t::F, __VA_ARGS__)
#define TrefMember(F) TrefMemberWithMeta(F, 0)
#define TrefMemberTypeWithMeta(F, ...) _TrefMember(F, (F**)0, __VA_ARGS__)
#define TrefMemberType(F, ...) TrefMemberTypeWithMeta(F, 0)

#define _TrefMember(F, val, ...) \
  _TrefPush(self_t, tref::imp::MemberTag, #F, val, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr auto is_pointer_to_pointer_v =
    is_pointer_v<T>&& is_pointer_v<remove_pointer_t<T>>;

/// recursive
template <class C, typename F>
constexpr bool each_member(F&& f, int level = 0) {
  auto next = each<C, MemberTag>([&](auto info) {
    // name, addr, meta
    return f(get<1>(info), get<2>(info), get<3>(info), level);
  });
  if (next)
    if constexpr (has_base_v<C>)
      return each_member<base_class_t<C>>(f, level + 1);
  return true;
}

/// recursive
template <class C, typename F>
constexpr bool each_subclass(F&& f, int level = 0) {
  return each<C, SubclassTag>([&](auto info) {
    using S = remove_pointer_t<tuple_element_t<1, decltype(info)>>;
    return f(get<1>(info), class_meta_v<S>, level) &&
           each_subclass<S>(f, level + 1);
  });
}
}  // namespace imp

using imp::base_class_t;
using imp::class_meta_v;
using imp::each_member;
using imp::each_subclass;
using imp::has_base_v;
using imp::is_pointer_to_pointer_v;
using imp::is_reflected_v;

}  // namespace tref
