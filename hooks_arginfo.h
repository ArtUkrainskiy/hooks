/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8468ab984d70f5d64aed041fcf47a72e31f653d7 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_hooks_set_hook, 0, 0, 3)
                ZEND_ARG_VARIADIC_INFO(0, clazz)
                ZEND_ARG_INFO(0, function)
                ZEND_ARG_INFO(0, before)
                ZEND_ARG_INFO(0, after)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(hooks_set_hook);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(hooks_set_hook, arginfo_hooks_set_hook)
	ZEND_FE_END
};
