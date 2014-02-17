include (plugin)

if (EXISTS ${CMAKE_SOURCE_DIR}/ObjectTypes/SplatCloud)
  add_definitions (-DENABLE_SPLATCLOUD_SUPPORT)
endif()

openflipper_plugin (DIRS PoissonReconstruction INSTALLDATA Icons )
