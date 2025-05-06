
set(LIBBACKTRACE_DIR .)
add_library(libbacktrace INTERFACE)
target_include_directories(libbacktrace INTERFACE "${LIBBACKTRACE_DIR}/include")
target_link_libraries(libbacktrace INTERFACE ${LIBBACKTRACE_LIBS} liblzma)