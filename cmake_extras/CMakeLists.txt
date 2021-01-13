
#Extra path for gitinfo.h file
#message("Building GitInfo.h")
#execute_process(COMMAND git log -1 --format=%at OUTPUT_VARIABLE GIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND git log -1 --format=%h OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND git log -1 --format=%H OUTPUT_VARIABLE GIT_REVISION_FULL OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND git log -1 --format=%D OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND git diff --shortstat OUTPUT_VARIABLE GIT_DIFF_SHORT_STAT OUTPUT_STRIP_TRAILING_WHITESPACE)
#string(TIMESTAMP COMPILATION_DATE "%s" )
#configure_file(GitInfo.h.in GitInfo.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GitInfo.h DESTINATION include)

#setup.sh/py/C files for SWAN notebooks
if(PRE_CONFIG)
   message("PRE_CONFIG: ${PRE_CONFIG} being added to ${CMAKE_INSTALL_PREFIX}/setup.{sh,py,C}")
endif()
set(AGRELEASE $ENV{AGRELEASE})
set(BIN_PATH ${CMAKE_INSTALL_PREFIX})
set(LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib)
set(INC_PATH ${CMAKE_INSTALL_PREFIX}/include)
configure_file(setup.sh.in ${CMAKE_CURRENT_BINARY_DIR}/setup.sh @ONLY)
configure_file(setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)
configure_file(setup.C.in ${CMAKE_CURRENT_BINARY_DIR}/setup.C @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup.sh ${CMAKE_CURRENT_BINARY_DIR}/setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.C DESTINATION .)

