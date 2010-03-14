project(QTICONLOADER)

add_library(QtIconLoader STATIC qticonloader.cpp qticonloader.h)
target_link_libraries(QtIconLoader ${QT_LIBRARIES})