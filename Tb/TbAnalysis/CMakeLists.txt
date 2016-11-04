################################################################################
# Package: TbAnalysis
################################################################################
gaudi_subdir(TbAnalysis v1r1)

gaudi_depends_on_subdirs(Tb/TbEvent
                         Tb/TbKernel
                         GaudiAlg)

find_package(ROOT COMPONENTS MathCore GenVector Physics Matrix)
find_package(Boost COMPONENTS iostreams)

gaudi_add_module(TbAnalysis
                 src/*.cpp
                 LINK_LIBRARIES TbEventLib TbKernelLib GaudiAlgLib Boost ROOT)

