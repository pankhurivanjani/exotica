cmake_minimum_required(VERSION 2.8.3)
project(exotica_time_indexed_rrt_connect_solver)

find_package(ompl REQUIRED)

find_package(catkin REQUIRED COMPONENTS
exotica_core
)

catkin_package(
  INCLUDE_DIRS include ${OMPL_INCLUDE_DIRS}
  LIBRARIES ${PROJECT_NAME} ${OMPL_LIBRARIES}
  CATKIN_DEPENDS exotica_core
)

AddInitializer(time_indexed_rrt_connect)
GenInitializers()

include_directories(include ${catkin_INCLUDE_DIRS} ${OMPL_INCLUDE_DIRS})
link_directories(${catkin_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} ${OMPL_LIBRARY_DIRS})

add_library(${PROJECT_NAME} src/time_indexed_rrt_connect.cpp)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${OMPL_LIBRARIES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_initializers ${catkin_EXPORTED_TARGETS})

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(DIRECTORY include/ DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
install(FILES exotica_plugins.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
