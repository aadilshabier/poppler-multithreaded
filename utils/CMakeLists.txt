set(common_srcs
  parseargs.cc
  Win32Console.cc
)
set(common_libs
  poppler
)

set(SERVICE_COMPONENTS s3)
find_package(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})

# pdftohtml
set(pdftohtml_SOURCES ${common_srcs}
  InMemoryFile.cc
  pdftohtml.cc
  HtmlFonts.cc
  HtmlLinks.cc
  HtmlOutputDev.cc
  nlohmann/json.hpp
  AWSHelper.cc
)
add_executable(pdftohtml ${pdftohtml_SOURCES})
target_link_libraries(pdftohtml ${common_libs} ${AWSSDK_LINK_LIBRARIES})
install(TARGETS pdftohtml DESTINATION bin)
install(FILES pdftohtml.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
