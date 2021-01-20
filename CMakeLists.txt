cmake_minimum_required(VERSION 3.8)
project(rdesktop_wrap)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH /home/user/Qt5.14.2/5.14.2/gcc_64) #这里需要填入自己安装的路径
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5Core)
find_package(Qt5Gui)
#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")
#创建工程文件
add_executable(rdesktop_wrap ${SRC_FILES} ${HEAD_FILES} ${RCC_FILES} ${WRAP_FILES})
target_link_libraries(rdesktop_wrap Qt5::Widgets Qt5::Network ssl crypto)
qt5_use_modules(rdesktop_wrap Core Widgets Network Gui)