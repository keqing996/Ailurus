
#include <cassert>

/* Assert - use comma operator,
 * need extra parentheses so the comma isn't treated as a delimiter between the arguments.
 * example: assert(("A must be equal to B", a == b));
 */
#ifdef NDEBUG
#   define ASSERT_MSG(expr, msg)
#else
#   define ASSERT_MSG(expr, msg) assert(((void)(msg), (expr)))
#endif