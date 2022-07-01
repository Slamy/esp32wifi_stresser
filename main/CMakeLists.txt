set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(SRCS
    "station_example_main.cpp"
    "wifi.c"
    INCLUDE_DIRS "."
)
