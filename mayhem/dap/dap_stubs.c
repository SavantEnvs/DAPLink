/* The board services DAP.c reaches through dap_strings.h; the probe under test has no target attached. */
#include <stddef.h>
#include "target_board.h"

const board_info_t g_board_info;

const char *info_get_unique_id(void)
{
    return "0240000032044e4500257009997b00386781000097969900";
}

const char *info_get_version(void)
{
    return "0254";
}
