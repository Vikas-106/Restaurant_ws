cmake_minimum_required(VERSION 3.0.2)
project(locations)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  std_msgs
)

catkin_package(
)


include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_executable(getlocation src/get_locations.cpp)
target_link_libraries(getlocation ${catkin_LIBRARIES} )

add_executable(uselocation src/use_location.cpp)
target_link_libraries(uselocation ${catkin_LIBRARIES} )
