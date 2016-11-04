################################################################################
# Package: TbIO
################################################################################
gaudi_subdir(TbIO v2r0)

gaudi_depends_on_subdirs(Tb/TbEvent
                         Tb/TbKernel
                         GaudiAlg)

find_package(Boost COMPONENTS iostreams)

gaudi_add_module(TbIO
                 src/*.cpp
                 LINK_LIBRARIES TbEventLib TbKernelLib GaudiAlgLib Boost)

