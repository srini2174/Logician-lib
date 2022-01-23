#
# EDSEngine
#
# This is the *nix master makefile for the C++ EDSEngine library
set(SOURCE_FILES "stdafx.cpp"
		 "ashost.cpp"
		 "Bimapper.cpp"
		 "Decode.cpp"
		 "EDSEngine.cpp"
		 "KnowledgeBase.cpp"
		 "RuleTable.cpp"
		 "TableSet.cpp"
		 "utilities.cpp"
		 )

set(HEADER_FILES "stdafx.h"
		 "ashost.h"
		 "Bimapper.h"
		 "Decode.h"
		 "IKnowledgeBase.h"
		 "KnowledgeBase.h"
		 "Numerics.h"
		 "RuleCell.h"
		 "RuleTable.h"
		 "TableSet.h"
		 "utilities.h"
		 "XMLWrapper.h")
		 	 
add_library(EDSEngine STATIC ${SOURCE_FILES} ${HEADER_FILES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-DUNICODE -D_UNICODE)
	target_link_libraries(EDSEngine ${Boost_LIBRARIES})
else()
	target_link_libraries(EDSEngine ${Boost_LIBRARIES} ${MOZJS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
endif()
add_subdirectory(EDSEngineTestApp)
