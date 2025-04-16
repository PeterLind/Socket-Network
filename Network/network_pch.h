#if !defined(WINVER)
#define WINVER 0x0605
#endif

//#if !defined(WIN32_LEAN_AND_MEAN)
//#define WIN32_LEAN_AND_MEAN
//#endif

#define IMGUI_DEFINE_MATH_OPERATORS

#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <fstream>
#include <array>
#include <exception>
#include <malloc.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <iostream>
#include <filesystem>
#include <any>
#include <limits>

#include <xmmintrin.h>
#include <comdef.h>
#include <assert.h>
#include <WinUser.h>
#include <wrl/client.h>
#include <windows.h>


#include <PropertyDefines.h>
#include <Profiler.h>
#undef WIN32_LEAN_AND_MEAN

//#define DEBUG_NEW new(__FILE__, __LINE__)
//#define new DEBUG_NEW
