add_library (Src SHARED
	    node.cpp	
		base_station.cpp 
		user_equipment.cpp 
		path.cpp 
		map.cpp
		channel.cpp
		tf_channel.cpp
		zak_channel.cpp
		channel_manager.cpp
		tf_channel_manager.cpp
		zak_channel_manager.cpp
		sysconfig.cpp)

target_link_libraries(Src -larmadillo -lconfig++)

target_include_directories (Src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
