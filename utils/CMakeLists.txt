
set(common_srcs
  parseargs.cc
  Win32Console.cc
)
set(common_libs
  poppler
)

# pdftohtml
set(pdftohtml_SOURCES ${common_srcs}
  InMemoryFile.cc
  pdftohtml.cc
  HtmlFonts.cc
  HtmlLinks.cc
  HtmlOutputDev.cc
  nlohmann/json.hpp
)
add_executable(pdftohtml ${pdftohtml_SOURCES})
target_link_libraries(pdftohtml ${common_libs})
install(TARGETS pdftohtml DESTINATION bin)
install(FILES pdftohtml.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
