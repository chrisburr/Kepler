################################################################################
# Package: TbAlignment
################################################################################
gaudi_subdir(TbAlignment v1r0)

gaudi_depends_on_subdirs(Tb/TbEvent
                         Tb/TbKernel
                         GaudiAlg)

find_package(ROOT COMPONENTS Minuit MathCore GenVector)
find_package(Boost COMPONENTS iostreams)

gaudi_add_module(TbAlignment
                 src/*.cpp
                 LINK_LIBRARIES TbEventLib TbKernelLib GaudiAlgLib Boost ROOT)

