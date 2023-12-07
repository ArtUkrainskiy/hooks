PHP_ARG_ENABLE([hooks],
  [whether to enable hooks support],
  [AS_HELP_STRING([--enable-hooks],
    [Enable hooks support])],
  [no])

if test "$PHP_HOOKS" != "no"; then
  AC_DEFINE(HAVE_HOOKS, 1, [ Have hooks support ])
  PHP_SUBST(HOOKS_SHARED_LIBADD)

  dnl Установим нужные флаги компилятора
  HOOKS_COMMON_FLAGS="-Wno-write-strings -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1"

  PHP_NEW_EXTENSION(hooks, , $ext_shared,,HOOKS_COMMON_FLAGS,cxx)

  dnl Собственно активация поддержки c++ c использованием стандарта с++17
  PHP_REQUIRE_CXX()
  PHP_CXX_COMPILE_STDCXX(17, mandatory, PHP_HOOKS_STDCXX)

  PHP_HOOKS_CXX_FLAGS="$HOOKS_COMMON_FLAGS $PHP_HOOKS_STDCXX"
  PHP_ADD_SOURCES_X(PHP_EXT_DIR(hooks),  hooks.cpp, $PHP_HOOKS_CXX_FLAGS, shared_objects_hooks, yes)
fi