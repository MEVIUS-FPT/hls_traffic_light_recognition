open_project -reset traffic_light_recognition_vitis_hls

add_files traffic_light_recognition.cpp
add_files traffic_light_recognition.h
add_files -tb traffic_light_recognition_tb.cpp

set_top traffic_light_recognition
open_solution -reset solution1
set_part {xck26-sfvc784-2LV-c}
create_clock -period 10 -name default

exit
