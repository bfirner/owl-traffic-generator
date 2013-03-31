SET(SourceFiles
  owl-traffic-generator.cpp
)

add_executable (owl-traffic-generator ${SourceFiles})
target_link_libraries (owl-traffic-generator owl-common owl-sensor)

INSTALL(TARGETS owl-traffic-generator RUNTIME DESTINATION bin/owl)
