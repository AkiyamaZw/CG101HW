if(WIN32)
include_directories("C:/Program Files/opencv/build/include" )
set(OpenCV_DIR "C:/Program Files/opencv/build" )
else()
include_directories(/usr/local/include )
endif()

find_package(OpenCV REQUIRED)

