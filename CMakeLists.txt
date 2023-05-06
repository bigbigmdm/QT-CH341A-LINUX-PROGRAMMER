cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(prog24 LANGUAGES CXX)

# Set the CMAKE_INSTALL_PREFIX to /usr if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "The default install prefix" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(LibUSB REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# ch341eeprom?
add_executable(${PROJECT_NAME}
	example/mainwindow.h
	example/optionsdialog.h
	ThirdParty/qhexedit2/src/qhexedit.h
	ThirdParty/qhexedit2/src/commands.h
    ThirdParty/qhexedit2/src/chunks.h
	example/searchdialog.h
	example/eepromsize.h
	example/programmer.h
	example/counter.h
	example/main.cpp
	example/mainwindow.cpp
	example/optionsdialog.cpp
	ThirdParty/qhexedit2/src/qhexedit.cpp
	ThirdParty/qhexedit2/src/commands.cpp
    ThirdParty/qhexedit2/src/chunks.cpp
	example/searchdialog.cpp
	example/eepromsize.cpp
	example/programmer.cpp
    example/${PROJECT_NAME}.qrc
	example/optionsdialog.ui
	example/searchdialog.ui
	example/eepromsize.ui
	example/programmer.ui
	example/translations/qhexedit_cs.ts
	example/translations/qhexedit_de.ts
	example/translations/qhexedit_ru.ts)

target_compile_definitions(${PROJECT_NAME} PRIVATE QHEXEDIT_EXPORTS)
target_include_directories(${PROJECT_NAME} PRIVATE ThirdParty/qhexedit2/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${LibUSB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${LibUSB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#install(FILES "${CMAKE_CURRENT_BINARY_DIR}/com.yuezk.qt.gpclient.metainfo.xml" DESTINATION share/metainfo)
#install(FILES "${CMAKE_CURRENT_BINARY_DIR}/com.yuezk.qt.gpclient.desktop" DESTINATION share/applications)
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/com.yuezk.qt.gpclient.svg" DESTINATION share/icons/hicolor/scalable/apps)
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/gp.conf" DESTINATION /etc/gpservice)

