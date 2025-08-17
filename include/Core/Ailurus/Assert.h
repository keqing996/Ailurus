
#include <cassert>

#ifdef NDEBUG
#   define ASSERT_MSG(expr, msg)
#else
#   define ASSERT_MSG(expr, msg) assert(((void)(msg), (expr)))
#endif