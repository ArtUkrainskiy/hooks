/* hooks extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <memory>
#include <iostream>
#include <TSRM/TSRM.h>
#include "php.h"
#include "ext/standard/info.h"
#include "php_hooks.h"
#include "hooks_arginfo.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_exceptions.h"
#include <zend_smart_str.h>
#include "ext/pdo/php_pdo_driver.h"

ZEND_DECLARE_MODULE_GLOBALS(hooks)

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif

/**
 * Функция на которую будет производиться подмена встроенных функций.
 * @param execute_data
 * @param return_value
 */
ZEND_NAMED_FUNCTION(my_hook) {
    auto function_name = execute_data->func->internal_function.function_name;
    std::string key;
    if (getThis() != nullptr) {
        key.append(execute_data->func->internal_function.scope->name->val);
        key.append("::");
    }
    key.append(function_name->val);
    auto hook_info_it = hooks_globals.originals->find(key);
    if (hook_info_it == hooks_globals.originals->end()) {
        return;
    }
    auto hook_info = hook_info_it->second;

    // Массив аргументов для callback-функции, будет содержать ключи name и args
    HashTable *handler_args_array;
    ALLOC_HASHTABLE(handler_args_array);
    zend_hash_init(handler_args_array, 8, NULL, ZVAL_PTR_DTOR, 0);

    // Аргументы для callback-функции, массив с одним элементом - массивом аргументов
    zval handler_args_zval[2];
    ZVAL_ARR(&handler_args_zval[0], handler_args_array);
    ZVAL_NULL(&handler_args_zval[1]);

    // Массив аргументов переданных при вызове оригинальной функции
    HashTable *hooked_args_array;
    ALLOC_HASHTABLE(hooked_args_array);
    zend_hash_init(hooked_args_array, 0, NULL, ZVAL_PTR_DTOR, 0);

    zval hooked_args_zval;
    ZVAL_ARR(&hooked_args_zval, hooked_args_array);

    // Копируем переданные аргументы, мы получим именно копию
    int arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    for (int i = 1; i <= arg_count; i++) {
        zval *arg = ZEND_CALL_ARG(execute_data, i);
        Z_TRY_ADDREF_P(arg);
        zend_hash_next_index_insert(hooked_args_array, arg);
    }

    if (getThis() != nullptr) {
        std::string sclass_name(execute_data->func->internal_function.scope->name->val);

        ZVAL_OBJ(&handler_args_zval[1], Z_OBJ_P(getThis()));
        if (sclass_name == "PDOStatement") {
            pdo_stmt_t *stmt = Z_PDO_STMT_P(getThis());
            if (stmt != nullptr && stmt->bound_params != nullptr) {
                HashTable *pdo_bound_params;
                ALLOC_HASHTABLE(pdo_bound_params);
                zend_hash_init(pdo_bound_params, 8, NULL, ZVAL_PTR_DTOR, 0);
                zval pdo_bound_params_zval;
                ZVAL_ARR(&pdo_bound_params_zval, pdo_bound_params);
                void *ptr;
                ZEND_HASH_FOREACH_PTR(stmt->bound_params, ptr) {
                            auto *data = static_cast<pdo_bound_param_data *>(ptr);
                            if (data != nullptr && data->name != nullptr) {
                                Z_TRY_ADDREF(data->parameter);
                                zend_hash_add(pdo_bound_params, data->name, &data->parameter);
                            }
                        } ZEND_HASH_FOREACH_END();
                zend_hash_str_add(handler_args_array, "pdo_bounds", 10, &pdo_bound_params_zval);
            }
        }
    }

    // Подготавливаем массив handler_args_array
    zval hook_name;
    ZVAL_STRING(&hook_name, key.c_str());
    zend_hash_str_add(handler_args_array, "name", 4, &hook_name);
    zend_hash_str_add(handler_args_array, "args", 4, &hooked_args_zval);

    // Производим вызов первой callback-функции, передавая в качестве аргумента массив
    hook_info->before_fci.params = handler_args_zval;
    hook_info->before_fci.param_count = 2;
    zval before_ret;
    hook_info->before_fci.retval = &before_ret;

    if (zend_call_function(&hook_info->before_fci, &hook_info->before_fcc) == FAILURE) {
        zend_hash_destroy(handler_args_array);
        FREE_HASHTABLE(handler_args_array);
        zval_ptr_dtor(&hook_name);
        zval_ptr_dtor(&hooked_args_zval);
        return;
    }

    // Вызов оригинальной функции
    hook_info->original_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    // Добавим результат работы оригинальной функции в массив аргументов
    if (Z_TYPE_P(return_value) != IS_NULL) {
        Z_TRY_ADDREF_P(return_value);
        zend_hash_str_add(handler_args_array, "result", 6, return_value);
    }

    // Проверим есть ли необработанное исключение и если есть, сохраним ссылку на него
    auto exception = EG(exception);
    if (exception) {
        zval exception_zval;
        ZVAL_OBJ(&exception_zval, exception);
        Z_TRY_ADDREF(exception_zval);
        zend_hash_str_add(handler_args_array, "exception", 9, &exception_zval);
        zend_exception_save();
    }

    // Производим вызов второй callback-функции, передавая в качестве аргумента массив
    hook_info->after_fci.params = handler_args_zval;
    hook_info->after_fci.param_count = 2;
    zval after_ret;
    hook_info->after_fci.retval = &after_ret;
    zend_call_function(&hook_info->after_fci, &hook_info->after_fcc);

    // Восстановим исключение
    if (exception) {
        zend_exception_restore();
    }

    // Освобождаем ресурсы
    zend_hash_destroy(handler_args_array);
    FREE_HASHTABLE(handler_args_array);
}


/**
 * Установка хуков для функции
 */
PHP_FUNCTION (hooks_set_hook) {
    // Инициализируем необходимые переменные для получения аргументов
    zend_string *function = nullptr;
    zend_class_entry *clazz = nullptr;
    zend_fcall_info before_fci = zend_fcall_info();
    zend_fcall_info after_fci = zend_fcall_info();
    zend_fcall_info_cache before_fcc = zend_fcall_info_cache();
    zend_fcall_info_cache after_fcc = zend_fcall_info_cache();

    // Проверяем соответствует ли текущий вызов одному из двух вариантов
    if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "CSff",
                                 &clazz, &function, &before_fci, &before_fcc, &after_fci, &after_fcc) != SUCCESS and
        zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(), "Sff",
                                 &function, &before_fci, &before_fcc, &after_fci, &after_fcc) != SUCCESS
            ) {
        // Если мы получили не те аргементы которые ожидали выбросим исключение и вернем false
        zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Invalid arguments");
        RETURN_FALSE;
    }

    // Получим указатель на оригинальную функцию или метод класса
    auto *original_function = static_cast<zend_function *>(zend_hash_str_find_ptr(
            clazz != nullptr ? &clazz->function_table : CG(function_table), function->val,
            function->len));
    // Проверим, что мы нашли встроенную функцию
    if (original_function != nullptr) {
        if (original_function->internal_function.type == ZEND_INTERNAL_FUNCTION) {
            // Сформируем ключ для std::map
            std::string key;
            if (clazz) {
                key.append(clazz->name->val);
                key.append("::");
            }
            key.append(function->val);

            //Проверим не установили ли мы хук ранее
            auto original_it = hooks_globals.originals->find(key);
            if (original_it == hooks_globals.originals->end()) {
                //сохраним все, что необходимо в глобальную переменную модуля
                auto hook = std::make_shared<hook_t>();
                hook->name = key;
                hook->before_fci = before_fci;
                hook->after_fci = after_fci;

                hook->before_fcc = before_fcc;
                hook->after_fcc = after_fcc;
                hook->original_handler = original_function->internal_function.handler;
                hooks_globals.originals->insert(std::pair<std::string, std::shared_ptr<hook_t>>(key, std::move(hook)));
                original_function->internal_function.handler = my_hook;
                RETURN_TRUE;
            }
        }
    }
    RETURN_FALSE;
}

static void php_init_globals(zend_hooks_globals *ng) {
    ng->originals = new std::unordered_map<std::string, std::shared_ptr<hook_t>>();
}

PHP_MINIT_FUNCTION (hooks) {
    ZEND_INIT_MODULE_GLOBALS(hooks, php_init_globals, null);
    return SUCCESS;
}

PHP_RINIT_FUNCTION (hooks) {
#if defined(ZTS) && defined(COMPILE_DL_HOOKS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}

PHP_MINFO_FUNCTION (hooks) {
    php_info_print_table_start();
    php_info_print_table_header(2, "hooks support", "enabled");
    php_info_print_table_end();
}

zend_module_entry hooks_module_entry = {
        STANDARD_MODULE_HEADER,
        "hooks",          // название расширения
        ext_functions,    // функции расширения
        PHP_MINIT(hooks),          // callback инициализации работы модуля
        nullptr,          // callback завершения работы модуля
        PHP_RINIT(hooks), // callback инициализации запроса
        nullptr,          // callback завершения запроса
        PHP_MINFO(hooks), // информация о модуле
        PHP_HOOKS_VERSION, // версия модуля
        STANDARD_MODULE_PROPERTIES
};
#ifdef COMPILE_DL_HOOKS
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(hooks)
#endif
