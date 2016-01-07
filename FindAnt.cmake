# - Try to find Ant
find_file(ANT_EXECUTABLE NAMES ant ant.sh ant.bat PATHS $ENV{ANT_HOME}/bin)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ANT DEFAULT_MSG ANT_EXECUTABLE)

MARK_AS_ADVANCED(ANT_EXECUTABLE)