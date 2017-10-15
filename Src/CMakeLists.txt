add_library (Src SHARED base_station.cpp user_equipment.cpp path.cpp map.cpp)

target_include_directories (Src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
