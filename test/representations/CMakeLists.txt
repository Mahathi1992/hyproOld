if (${STATICLIB_SWITCH} STREQUAL "OFF")

    # reduce compiler optimization for tests
    set(representation_test_sources
            BoxTest.cpp
            CarlPolytopeTest.cpp
            ConverterTest.cpp
            DifferenceBoundsTest.cpp
            GridTest.cpp
            HPolytopeTest.cpp
            NeighborhoodContainerTest.cpp
            OrthogonalPolyhedronTest.cpp
            PolytopeSupportFunctionTest.cpp
            SupportFunctionTest.cpp
            #TaylorModelTest.cpp
            VPolytopeTest.cpp
            ZonotopeTest.cpp
            SupportFunctionNewTest.cpp
            TemplatePolyhedronTest.cpp
            #OrthoplexTest.cpp
            )

    if (HYPRO_USE_PPL)
        set(representation_test_sources "${representation_test_sources} PolytopeTest.cpp PolytopeUtilTest.cpp")
    endif ()

    SET_SOURCE_FILES_PROPERTIES(${representation_test_sources} PROPERTIES COMPILE_FLAGS -O0)

    ################################
    # Unit Tests
    ################################

    add_executable(runRepresentationTests ${representation_test_sources})

    add_dependencies(runRepresentationTests hypro_resources)
    target_include_directories(runRepresentationTests PRIVATE ${GTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/test/include/)

    target_link_libraries(runRepresentationTests PUBLIC
            ${GTEST_LIBRARIES}
            ${PROJECT_NAME}-static
            )

    cotire(runRepresentationTests)

    add_test(NAME representations COMMAND runRepresentationTests)

    if (DEVELOPER)
        set_property(TEST representations PROPERTY
                ENVIRONMENT LD_PRELOAD=${asanLocation}
                )
    endif ()

    add_dependencies(allTests runRepresentationTests)
endif ()
