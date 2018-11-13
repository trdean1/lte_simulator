add_executable (static_zak static_zak.cpp)
add_executable (static_tf static_tf.cpp)

target_link_libraries (static_zak LINK_PUBLIC Src)
target_link_libraries (static_tf LINK_PUBLIC Src)
