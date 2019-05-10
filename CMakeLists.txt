cmake_minimum_required(VERSION 3.6)

project("pinotams")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wno-unused-function")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if (NOT DEFINED SHARE_PREFIX)
  set(SHARE_PREFIX share/pinotams/)
endif (NOT DEFINED SHARE_PREFIX)
if (NOT DEFINED ETC_PREFIX)
  set(ETC_PREFIX etc/)
endif (NOT DEFINED ETC_PREFIX)
if (NOT DEFINED VAR_PREFIX)
  set(VAR_PREFIX var/pinotams/)
endif (NOT DEFINED VAR_PREFIX)
set(CONFIG_FILE ${CMAKE_INSTALL_PREFIX}/${ETC_PREFIX}pinotams.conf)
set(CACHE_FILE ${CMAKE_INSTALL_PREFIX}/${VAR_PREFIX}pinotams.db)
configure_file(config.h.in config.h)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(ConfParser
  conf_file.y
  ${CMAKE_CURRENT_BINARY_DIR}/conf_file.parser.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/conf_file.parser.h)
FLEX_TARGET(ConfLexer
  conf_file.l
  ${CMAKE_CURRENT_BINARY_DIR}/conf_file.lexer.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/conf_file.lexer.h)
ADD_FLEX_BISON_DEPENDENCY(ConfLexer ConfParser)

include_directories(pinotams ${CMAKE_BINARY_DIR})
add_executable(pinotams
  pinotams.c
  notams.c
  config.c
  ${BISON_ConfParser_OUTPUTS}
  ${FLEX_ConfLexer_OUTPUTS})
target_link_libraries(pinotams curl jansson sqlite3 etpan pcre2-8)

install(TARGETS pinotams DESTINATION bin)
install(FILES pinotams.conf.example DESTINATION ${ETC_PREFIX})
