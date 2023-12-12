# idf_component_register(SRCS "hello_world_main.c"
#                     INCLUDE_DIRS "")


file(GLOB_RECURSE SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
INCLUDE_DIRS 
"./rn8209c_sdk/include"
)