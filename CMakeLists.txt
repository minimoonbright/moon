cmake_minimum_required(VERSION 3.20)
project(SanguoRPG VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

set(SOURCES
    src/main.cpp
    src/core/Types.cpp
    src/core/DataManager.cpp
    src/core/SaveManager.cpp
    src/core/GameEngine.cpp
    src/map/MapScene.cpp
    src/map/MapView.cpp
    src/map/PlayerSprite.cpp
    src/map/NpcSprite.cpp
    src/map/MonsterSprite.cpp
    src/battle/BattleEngine.cpp
    src/battle/BattleWidget.cpp
    src/character/Character.cpp
    src/recruit/RecruitSystem.cpp
    src/ui/MainWindow.cpp
    src/ui/BuildingWidget.cpp
    src/ui/EquipWidget.cpp
    src/ui/TeamWidget.cpp
)

set(HEADERS
    src/core/Types.h
    src/core/DataManager.h
    src/core/SaveManager.h
    src/core/GameEngine.h
    src/map/MapScene.h
    src/map/MapView.h
    src/map/PlayerSprite.h
    src/map/NpcSprite.h
    src/map/MonsterSprite.h
    src/battle/BattleEngine.h
    src/battle/BattleWidget.h
    src/character/Character.h
    src/recruit/RecruitSystem.h
    src/ui/MainWindow.h
    src/ui/BuildingWidget.h
    src/ui/EquipWidget.h
    src/ui/TeamWidget.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Multimedia)

# 将 data/ 复制到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
)
