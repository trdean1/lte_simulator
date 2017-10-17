add_library (Src SHARED
	    node.cpp	
		base_station.cpp 
		user_equipment.cpp 
		path.cpp 
		map.cpp
		channel.cpp
		tf_channel.cpp)

target_include_directories (Src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
