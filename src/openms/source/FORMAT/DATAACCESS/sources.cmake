# src/openms/source/FORMAT/DATAACCESS/sources.cmake
#---------------------------------------------
# Collect all source files in the DATAACCESS folder
#---------------------------------------------

# Directory of this .cmeake file (holds the .cpp files)
set(DATAACCESS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# List of all .cpp files in DATAACCESS
set(DATAACCESS_SOURCES
  MSDataWritingConsumer.cpp
  MSDataTransformingConsumer.cpp
  MSDataAggregatingConsumer.cpp
  MSDataCachedConsumer.cpp
  MSDataChainingConsumer.cpp
  MSDataStoringConsumer.cpp
  MSDataSqlConsumer.cpp
  NoopMSDataConsumer.cpp
  SiriusFragmentAnnotation.cpp
  SwathFileConsumer.cpp
)

# Prepend full path to each filename
list(TRANSFORM DATAACCESS_SOURCES PREPEND "${DATAACCESS_DIR}/")

# Append these files into the global OpenMS_sources list
list(APPEND OpenMS_sources ${DATAACCESS_SOURCES})

# Organize for IDEs: group these under Source Files\FORMAT\DATAACCESS
source_group("Source Files\\FORMAT\\DATAACCESS" FILES ${DATAACCESS_SOURCES})


