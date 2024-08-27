cmake_minimum_required(VERSION 3.16)
project(hooks CXX)

SET(CMAKE_CXX_STANDARD 17)
SET(PHPCPP_PHP_PATH "/usr/include/php/20210902")

INCLUDE_DIRECTORIES(
        "${PHPCPP_PHP_PATH}/"
        "${PHPCPP_PHP_PATH}/main"
        "${PHPCPP_PHP_PATH}/Zend"
        "${PHPCPP_PHP_PATH}/TSRM"
        "${PHPCPP_PHP_PATH}/ext"
        "${PHPCPP_PHP_PATH}/build/main"
        "${PHPCPP_PHP_PATH}/build/Zend"
)

add_library(hooks SHARED
        config.h
        hooks.cpp
        php_hooks.h)
