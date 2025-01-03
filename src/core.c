#include "core.h"

// -- String -------------------------------------------------------------------
// Length based strings.

str_t str(const u8* data, u32 len) {
    return (str_t) { data, len };
}
