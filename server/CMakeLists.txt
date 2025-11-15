cmake_minimum_required(VERSION 3.21)
project(server)

set(CMAKE_VERBOSE_MAKEFILE ON)

function(force_redefine_file_macro_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach (sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
                PROPERTY COMPILE_DEFINITIONS)
        # Get the relative path of the source file in project directory
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # Set the updated compile definitions on the source file.
        set_property(
                SOURCE "${sourcefile}"
                PROPERTY COMPILE_DEFINITIONS ${defs}
        )
    endforeach ()
endfunction()

# 指定编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17 -O0 -ggdb -Wall -Werror -Wno-unused-result")

# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
# -fPIC: 生成位置无关的代码，便于动态链接
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   \
    -Wno-unused-function                  \
    -Wno-builtin-macro-redefined          \
    -Wno-deprecated                       \
    -Wno-deprecated-declarations"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/sylar SYLAR_SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/example/day1 SRC_DAY1)
add_executable(server1 ${SRC_DAY1} ${SYLAR_SRCS})
force_redefine_file_macro_for_sources(server1)

include_directories(${PROJECT_PATH}/boost/include)

set(LIBS pthread
        dl
        unwind
        unwind-generic
)

file(GLOB BOOST_LIBS "${PROJECT_PATH}/boost/lib/lib*.so")

target_link_libraries(server1 PRIVATE ${BOOST_LIBS} ${LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)