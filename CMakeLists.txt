check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

find_library(openfec openfec)

build_lib(
    LIBNAME al-fec
    SOURCE_FILES model/al-fec-codec.cc
                 model/al-fec-header.cc
                 model/al-fec-codec-openfec-rs.cc
    HEADER_FILES model/al-fec-codec.h
                 model/al-fec-header.h
                 model/al-fec-codec-openfec-rs.h
    LIBRARIES_TO_LINK ${libcore}
                      ${openfec}
    TEST_SOURCES test/al-fec-test-codec-openfec-rs.cc
)
    
