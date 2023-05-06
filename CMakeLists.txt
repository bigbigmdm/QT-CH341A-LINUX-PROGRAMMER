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
	include/mainwindow.h
	include/optionsdialog.h
	ThirdParty/qhexedit2/src/qhexedit.h
	ThirdParty/qhexedit2/src/commands.h
    ThirdParty/qhexedit2/src/chunks.h
	include/searchdialog.h
	include/eepromsize.h
	include/programmer.h
	include/counter.h
	src/main.cpp
	src/mainwindow.cpp
	src/optionsdialog.cpp
	ThirdParty/qhexedit2/src/qhexedit.cpp
	ThirdParty/qhexedit2/src/commands.cpp
    ThirdParty/qhexedit2/src/chunks.cpp
	src/searchdialog.cpp
	src/eepromsize.cpp
	src/programmer.cpp
    ${PROJECT_NAME}.qrc
	src/optionsdialog.ui
	src/searchdialog.ui
	src/eepromsize.ui
	src/programmer.ui
	translations/qhexedit_cs.ts
	translations/qhexedit_de.ts
	translations/qhexedit_ru.ts)

target_compile_definitions(${PROJECT_NAME} PRIVATE QHEXEDIT_EXPORTS)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ThirdParty/qhexedit2/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${LibUSB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${LibUSB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/Prog24cXX.desktop" DESTINATION share/applications)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/24Cxx_icon32.png" DESTINATION share/icons/hicolor/scalable/apps)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/24Cxx_icon48.png" DESTINATION share/icons/hicolor/scalable/apps)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/24Cxx_icon64.ico" DESTINATION share/icons/hicolor/scalable/apps)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/24Cxx_icon64.png" DESTINATION share/icons/hicolor/scalable/apps)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/99-CH341.rules" DESTINATION /etc/udev/rules.d)

