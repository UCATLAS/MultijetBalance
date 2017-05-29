################################################################################
# Package: InsituBalance
################################################################################

# Declare the package name:
atlas_subdir( InsituBalance )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          xAODAnaHelpers
)

# Find the needed external(s):
find_package( ROOT COMPONENTS Core RIO Hist Tree )

# build a dictionary for the library
atlas_add_root_dictionary ( InsituBalanceLib InsituBalanceDictSource
                            ROOT_HEADERS InsituBalance/*.h Root/LinkDef.h
                            EXTERNAL_PACKAGES ROOT
)

# build a shared library
atlas_add_library( InsituBalanceLib InsituBalance/*.h Root/*.cxx ${InsituBalanceDictSource}
#atlas_add_library( InsituBalanceLib InsituBalance/*.h Root/*.cxx util/*.cxx ${InsituBalanceDictSource}
                   PUBLIC_HEADERS InsituBalance
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} 
                   xAODAnaHelpersLib JetTileCorrectionLib JES_ResponseFitterLib
)
# Executable(s) in the package:
atlas_add_executable( MJBFit
   util/MJBFit.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} JES_ResponseFitterLib )

# Install files from the package:
atlas_install_scripts( scripts/*.py )
atlas_install_data( data/* )
