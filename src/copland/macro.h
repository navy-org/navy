#pragma once 

#define loop for(;;)
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define PACKED [[gnu::packed]]
#define MAYBE_UNUSED [[maybe_unused]]