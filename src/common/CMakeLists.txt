add_subdirectory(json)

add_library(common asioserver.cpp connectionset.cpp event.cpp httprequest.cpp)
target_link_libraries(common ${CURL_LIBRARIES} )
