idf_component_register(SRCS "gatts_demo.c" "vehicleControl.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
