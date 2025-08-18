set(SOURCES execution_manager.cpp check_condition.cpp executor_explain.cpp executor_semi_join.cpp executor_anti_join.cpp)
add_library(execution STATIC ${SOURCES})

target_link_libraries(execution system record transaction planner)