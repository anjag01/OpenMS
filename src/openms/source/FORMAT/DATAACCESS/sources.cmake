# src/openms/source/FORMAT/DATAACCESS/sources.cmake
#---------------------------------------------
# DATAACCESS backend source files for libOpenMS
#---------------------------------------------

# List of source files relative to src/openms directory
set(DATAACCESS_SOURCES
  "source/FORMAT/DATAACCESS/MSDataWritingConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataTransformingConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataAggregatingConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataCachedConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataChainingConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataStoringConsumer.cpp"
  "source/FORMAT/DATAACCESS/MSDataSqlConsumer.cpp"
  "source/FORMAT/DATAACCESS/NoopMSDataConsumer.cpp"
  "source/FORMAT/DATAACCESS/SiriusFragmentAnnotation.cpp"
  "source/FORMAT/DATAACCESS/SwathFileConsumer.cpp"
)

# Append these files into the global OpenMS_sources list for compilation
list(APPEND OpenMS_sources ${DATAACCESS_SOURCES})

# Organize for IDEs: group these under Source Files\FORMAT\DATAACCESS
source_group("Source Files\\FORMAT\\DATAACCESS" FILES ${DATAACCESS_SOURCES})



