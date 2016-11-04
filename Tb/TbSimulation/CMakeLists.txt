################################################################################
# Package: TbSimulation
################################################################################
gaudi_subdir(TbSimulation v1r3)

gaudi_depends_on_subdirs(Tb/TbEvent
                         Tb/TbKernel
                         GaudiAlg)

find_package(ROOT COMPONENTS MathCore)

gaudi_add_module(TbSimulation
                 src/*.cpp
                 LINK_LIBRARIES TbEventLib TbKernelLib GaudiAlgLib ROOT)

