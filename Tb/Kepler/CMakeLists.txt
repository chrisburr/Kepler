################################################################################
# Package: Kepler 
################################################################################
gaudi_subdir(Kepler v3r0)

gaudi_depends_on_subdirs(Tb/TbAlgorithms
                         Tb/TbIO
                         Tb/TbKernel
                         Gaudi
                         GaudiConf
                         GaudiKernel)

gaudi_install_python_modules()

gaudi_env(SET KEPLEROPTS \${KEPLERROOT}/options)

gaudi_add_test(QMTest QMTEST)
