cmake_minimum_required(VERSION 3.20)
project(AIPredictiveScheduler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GUI "Build the Qt GUI application" ON)

# Enable testing
enable_testing()

# Set up include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find Eigen3
find_package(Eigen3 CONFIG REQUIRED)

# Core Module Sources
set(CORE_SOURCES
    src/ML/ProcessGenerator.cpp
    src/ML/FeatureExtractor.cpp
    src/ML/LinearRegressor.cpp
    src/ML/Predictor.cpp
    src/Scheduler/MetricsEngine.cpp
    src/Scheduler/FCFSScheduler.cpp
    src/Scheduler/SJFScheduler.cpp
    src/Scheduler/RoundRobinScheduler.cpp
    src/Scheduler/MLScheduler.cpp
)

# Core Library (shared across tests and main executable)
add_library(CoreLib STATIC ${CORE_SOURCES})
target_include_directories(CoreLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(CoreLib PUBLIC Eigen3::Eigen)

# C++ CLI Simulator for Python wrapper
add_executable(SimulatorCLI src/cli.cpp src/simulator.cpp)
target_include_directories(SimulatorCLI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(SimulatorCLI PRIVATE Threads::Threads)

# We will add the main executable here later (Steps 6-10)
if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets Charts REQUIRED)

    # Enable automatic Qt build tools
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    # GUI and Application Source Files
    set(GUI_SOURCES
        src/GUI/MainWindow.cpp
        src/GUI/GanttWidget.cpp
        src/GUI/DashboardWidget.cpp
        src/main.cpp
    )

    # Main Executable
    add_executable(SchedulerApp ${GUI_SOURCES})
    target_include_directories(SchedulerApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(SchedulerApp PRIVATE CoreLib Qt6::Widgets Qt6::Charts)
endif()

# Add Tests directory
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
