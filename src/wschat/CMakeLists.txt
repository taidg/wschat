cmake_minimum_required (VERSION 2.6)

set(CHAT_SOURCES chatserver.cpp user.cpp)
set(CHAT_HEADERS chatserver.h user.h chatasioserver.h)
add_executable(chatserver ${CHAT_SOURCES} ${CHAT_HEADERS} )

target_link_libraries(chatserver ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} json common)

add_custom_command(TARGET chatserver PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/chatserver/config $<TARGET_FILE_DIR:chatserver>/config )
