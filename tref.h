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
tuple<Id<0>> __refElem(C**, T, Id<0> id);

#define _RefElemCnt(C, T)                                 \
  std::tuple_element_t<0, decltype(__refElem((C**)0, T{}, \
                                             tref::imp::MaxId{}))>::value

#define _RefNextId(C, T) tref::imp::Id<_RefElemCnt(C, T) + 1>

#define _RefReturn(T) \
  ->decltype(T) { return T; }

#define _RefPush(C, T, ...)                                    \
  friend constexpr auto __refElem(C**, T, _RefNextId(C, T) id) \
      _RefReturn(std::make_tuple(id, __VA_ARGS__))

template <class C, class T, class F, size_t... Is>
constexpr bool fold(index_sequence<Is...>, F&& f) {
  return (f(__refElem((C**)nullptr, T{}, Id<Is>{})) && ...);
}

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
};

template <typename C, typename T, typename F>
constexpr bool each(F f) {
  constexpr auto cnt = _RefElemCnt(C, T);
  if constexpr (cnt > 0) {
    return fold<C, T>(tail(make_index_sequence<cnt + 1>{}), f);
  } else
    return true;
};

//////////////////////////////////////////////////////////////////////////

/// super trait

void HasSuper(void*);

#define _RefHasSuper(T, S) \
  friend constexpr S* HasSuper(T**) { return nullptr; }

template <typename T>
constexpr auto has_super_v = !std::is_same_v<decltype(HasSuper((T**)0)), void>;

template <typename T>
using super_class_t = remove_pointer_t<decltype(HasSuper((T**)0))>;

//////////////////////////////////////////////////////////////////////////

struct MemberTag {};
struct SubclassTag {};
false_type __meta(void*);

#define RefTypeRoot(T) _RefTypeCommon(T)

#define RefType(T)       \
 private:                \
  using super = _parent; \
  _RefTypeCommon(T);     \
  _RefSuper(super);

#define _RefSuper(super)     \
  _RefHasSuper(self, super); \
  _RefPush(super, tref::imp::SubclassTag, (self*)0)

#define _RefTypeCommon(T) \
  using self = T;         \
                          \
 protected:               \
  using _parent = T;      \
                          \
 public:                  \
  friend constexpr auto __meta(T**) { return std::make_tuple(#T, sizeof(T)); }

#define _Reflected(F, val, ...) \
  _RefPush(self, tref::imp::MemberTag, #F, val, __VA_ARGS__)

#define RefMemberWithMeta(F, ...) _Reflected(F, &self::F, __VA_ARGS__)
#define RefMember(F) RefMemberWithMeta(F, nullptr)
#define RefMemberTypeWithMeta(F, ...) _Reflected(F, (F**)0, __VA_ARGS__)
#define RefMemberType(F, ...) RefMemberTypeWithMeta(F, nullptr)

//////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr auto is_reflected_v =
    !std::is_same_v<decltype(__meta((T**)0)), false_type>;

template <typename T>
constexpr auto class_meta_v = __meta((T**)0);

template <typename T>
constexpr auto is_pointer_to_pointer_v =
    is_pointer_v<T>&& is_pointer_v<remove_pointer_t<T>>;

/// recursive
template <class C, typename F>
constexpr bool eachMember(F&& f, int level = 0) {
  auto next = each<C, MemberTag>([&](auto info) {
    // name, addr, meta
    return f(get<1>(info), get<2>(info), get<3>(info), level);
  });
  if (next)
    if constexpr (has_super_v<C>)
      return eachMember<super_class_t<C>>(f, level + 1);
  return true;
}

/// recursive
template <class C, typename F>
constexpr bool eachSubclass(F&& f, int level = 0) {
  return each<C, SubclassTag>([&](auto info) {
    using S = remove_pointer_t<tuple_element_t<1, decltype(info)>>;
    return f(get<1>(info), class_meta_v<S>, level) &&
           eachSubclass<S>(f, level + 1);
  });
}
}  // namespace imp

using imp::class_meta_v;
using imp::eachMember;
using imp::eachSubclass;
using imp::has_super_v;
using imp::is_pointer_to_pointer_v;
using imp::is_reflected_v;
using imp::super_class_t;

}  // namespace tref
