################################################################################
# Package: TbKernel
################################################################################
gaudi_subdir(TbKernel v3r0)

gaudi_depends_on_subdirs(Tb/TbEvent
                         GaudiAlg)

find_package(ROOT COMPONENTS GenVector)
find_package(Boost)

gaudi_add_library(TbKernelLib
                  src/lib/*.cpp
                  PUBLIC_HEADERS TbKernel
                  LINK_LIBRARIES GaudiAlgLib)

gaudi_add_module(TbKernel
                 src/*.cpp
                 INCLUDE_DIRS AIDA Boost
                 LINK_LIBRARIES Boost GaudiAlgLib TbKernelLib TbEventLib ROOT)

gaudi_install_python_modules()

