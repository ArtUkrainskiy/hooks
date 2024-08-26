/* hooks extension for PHP */

#ifndef PHP_HOOKS_H
# define PHP_HOOKS_H

#include <string>
#include <unordered_map>

extern zend_module_entry hooks_module_entry;
# define phpext_hooks_ptr &hooks_module_entry

# define PHP_HOOKS_VERSION "0.1.0"


struct hook_t {
    std::string name;

    zend_fcall_info before_fci{};
    zend_fcall_info after_fci{};

    zend_fcall_info_cache before_fcc{};
    zend_fcall_info_cache after_fcc{};

    zif_handler original_handler;
};


ZEND_BEGIN_MODULE_GLOBALS(hooks)
    std::unordered_map<std::string, std::shared_ptr<hook_t>> *originals;
ZEND_END_MODULE_GLOBALS(hooks)



# if defined(ZTS) && defined(COMPILE_DL_HOOKS)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_HOOKS_H */
