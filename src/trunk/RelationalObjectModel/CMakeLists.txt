#
# RelationalObjectModel
#
# This is the *nix master makefile for the C++ RelationalObjectModel library
set(SOURCE_FILES "stdafx.cpp"
		 "ROM.cpp"
		 "ROMNode.cpp"
		 "ROMDictionary.cpp"		 
		 "LinearEngine.cpp"
		 "utilities.cpp"
		 )

set(HEADER_FILES "stdafx.h"
		 "ROMNode.h"
		 "ROMInterfaces.h"
		 "ROMDictionaryAttribute.h"
		 "ROMDictionary.h"		 
		 "LinearEngine.h"
		 "utilities.h")
		 
add_definitions(-DUNICODE -D_UNICODE)
		 
add_library(RelationalObjectModel STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(RelationalObjectModel
			EDSEngine
			${LIBXSLT_LIBRARIES})
						
add_subdirectory(ROMAppConsoleTest)
