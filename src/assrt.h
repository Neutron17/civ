#ifndef _NTR_ASSERT_H_
#define _NTR_ASSERT_H_ 1
#include <stdbool.h>

void __assert_m(bool expr, const char *restrict msg, const char *restrict file, const char *restrict func, int line);
#define ASSERT_M(EXPR, MSG) __assert_m(EXPR, MSG, __FILE__, __func__, __LINE__)
#define ASSERT(EXPR) ASSERT_M(EXPR, NULL)

#endif /* ifndef _NTR_ASSERT_H_ */

