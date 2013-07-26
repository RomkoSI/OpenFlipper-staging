
# Include Testing package
if(BUILD_TESTING)

  include( ${CMAKE_SOURCE_DIR}/tests/testGenerators.cmake )

  # ========================================================================
  # ========================================================================
  # Tests
  # ========================================================================
  # ========================================================================
  run_single_object_file_mesh_test("assimpplugin" "Plugin-Assimp/cube.xgl" "fileMeshTest.ofs")
  run_single_object_file_mesh_test("assimpplugin" "Plugin-Assimp/cube.dae" "fileMeshTest.ofs")

endif()
