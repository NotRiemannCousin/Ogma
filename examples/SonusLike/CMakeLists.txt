add_executable(SonusExample main.cpp)
target_include_directories(SonusExample PRIVATE include)
target_link_libraries(SonusExample PRIVATE Ogma)
