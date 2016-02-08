# FindQtIFW
# ---------
#
# Find Qt Installer Framework, this module defines:
#
# QtIFW_CREATOR, where to find binarycreator.exe
# QtIFW_FOUND, if the InnoSetup installation was found
#

find_program(
       QtIFW_CREATOR
       NAMES binarycreator
       DOC "QtIFW binarycreator executable"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
       QtIFW
       FOUND_VAR QtIFW_FOUND
       REQUIRED_VARS QtIFW_CREATOR
)

mark_as_advanced(QtIFW_CREATOR)