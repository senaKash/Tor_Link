cmake_minimum_required(VERSION 3.5)

# Имя проекта и версия
project(Tor_Link VERSION 0.1)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Проверяем наличие Qt
find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)

# Указываем путь к OpenSSL вручную
set(OPENSSL_ROOT_DIR "C:/Program Files/FireDaemon OpenSSL 3")
set(OPENSSL_INCLUDE_DIR "C:/Program Files/FireDaemon OpenSSL 3/include")
set(OPENSSL_LIBRARIES "C:/Program Files/FireDaemon OpenSSL 3/lib")

# Найти пакет OpenSSL
find_package(OpenSSL REQUIRED)

# Добавляем исходные файлы проекта
set(SOURCES
    main.cpp
    MainWindow.cpp
    TorController.cpp
)

set(HEADERS
    MainWindow.h
    TorController.h
)

set(UI_FILES
    MainWindow.ui
)

# Генерируем файлы UI и заголовки для слотов/сигналов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



# Добавляем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# Указываем зависимости от Qt5 (Core, Widgets, Network) с ключевым словом PRIVATE
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)

# Добавляем OpenSSL зависимости с ключевым словом PRIVATE
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})

# Указываем путь к заголовочным файлам
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})



# Создание файла конфигурации torrc (если нужно создать его автоматически)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/torrc ${CMAKE_BINARY_DIR}/torrc COPYONLY)

# Копирование бинарного файла Tor в выходную директорию (если tor.exe находится рядом с проектом)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/tor.exe"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
