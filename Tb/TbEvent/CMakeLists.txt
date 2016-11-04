################################################################################
# Package: TbEvent
################################################################################
gaudi_subdir(TbEvent v2r2)

gaudi_depends_on_subdirs(Event/EventBase
                         GaudiObjDesc)

find_package(GSL)

include(GaudiObjDesc)

god_build_headers(xml/*.xml)

gaudi_add_library(TbEventLib src/*.cpp
                  NO_PUBLIC_HEADERS
                  INCLUDE_DIRS GSL 
                  LINK_LIBRARIES GSL GaudiKernel)

god_build_dictionary(xml/*.xml
                     INCLUDE_DIRS GSL Event/EventBase
                     LINK_LIBRARIES GSL GaudiObjDescLib TbEventLib)

