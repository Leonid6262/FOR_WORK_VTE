#pragma once

//#include <stdlib.h> 
//extern "C" struct mallinfo __iar_dlmallinfo(void); 
//extern "C" void __iar_dlmalloc_stats(void);

#include "Ports_init.hpp"
#include "emc.hpp"
#include "Timers_init.hpp"
#include "AdcStorage.hpp"
#include "dIOStorage.hpp"
#include "IntPriority.hpp" 
#include "pause_us.hpp"
#include "factory.hpp"

#if __cplusplus >= 202002L
#pragma message("C++20")
#elif __cplusplus >= 201703L
#pragma message("C++17")
#elif __cplusplus >= 201402L
#pragma message("C++14")
#elif __cplusplus >= 201103L
#pragma message("C++11")
#else
#pragma message("C++98/03")
#endif