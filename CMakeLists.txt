cmake_minimum_required(VERSION 3.5)

# Set the project name
project (dds_samples)

set(CMAKE_CXX_FLAGS "-O2 -Wall -g -std=c++17")
set(CMAKE_BUILD_TYPE Release)

message("current dir:",${PROJECT_SOURCE_DIR})

# find_package(CycloneDDS REQUIRED)
set(DDS_DIR /home/li/cyclonedds-install)
include_directories(${DDS_DIR}/include)
link_directories(${DDS_DIR}/lib)

add_executable(pub_samples src/pub.cpp src/MindosData.c)
target_link_libraries(pub_samples pthread glog ddsc)

add_executable(sub_samples src/sub.cpp src/MindosData.c)
target_link_libraries(sub_samples pthread glog ddsc)

add_executable(sub_waitset_samples src/sub_waitset.cpp src/MindosData.c)
target_link_libraries(sub_waitset_samples pthread glog ddsc)

add_executable(sub_class_samples src/sub_class_waitset.cpp src/MindosData.c)
target_link_libraries(sub_class_samples pthread glog ddsc)

add_executable(request_samples src/request.cpp src/MindosData.c)
target_link_libraries(request_samples pthread glog ddsc)

add_executable(response_samples src/response.cpp src/MindosData.c)
target_link_libraries(response_samples pthread glog ddsc)

add_executable(request_class_samples src/request_class.cpp src/MindosData.c)
target_link_libraries(request_class_samples pthread glog ddsc)

add_executable(response_class_samples src/response_class.cpp src/MindosData.c)
target_link_libraries(response_class_samples pthread glog ddsc)