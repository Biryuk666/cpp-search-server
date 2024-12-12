cmake_minimum_required(VERSION 3.11)

project(SearchServer CXX)
set(CMAKE_CXX_STANDARD 17)

set(SEARCHSERVER_MAIN_FILES "search-server/search_server.h" "search-server/search_server.cpp")
set(SEARCHSERVER_SUBFILES 
    "search-server/document.h" "search-server/document.cpp"
    "search-server/paginator.h"
    "search-server/process_queries.h" "search-server/process_queries.cpp"
    "search-server/read_input_functions.h" "search-server/read_input_functions.cpp"
    "search-server/remove_duplicates.h" "search-server/remove_duplicates.cpp"
    "search-server/request_queue.h" "search-server/request_queue.cpp"
    "search-server/string_processing.h" "search-server/string_processing.cpp"
    "search-server/test_example_functions.h" "search-server/test_example_functions.cpp")

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_executable(search_server "search-server/main.cpp" ${SEARCHSERVER_MAIN_FILES} 
${SEARCHSERVER_SUBFILES})