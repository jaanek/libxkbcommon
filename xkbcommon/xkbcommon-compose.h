/*
 * Copyright © 2013 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef XKBCOMMON_COMPOSE_H
#define XKBCOMMON_COMPOSE_H

#include <xkbcommon/xkbcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

struct xkb_compose;
struct xkb_compose_state;

enum xkb_compose_compile_flags {
    XKB_COMPOSE_COMPILE_NO_FLAGS = 0
};

enum xkb_compose_format {
    XKB_COMPOSE_FORMAT_TEXT_V1 = 1
};

struct xkb_compose *
xkb_compose_new_from_file(struct xkb_context *context,
                          FILE *file,
                          const char *locale,
                          enum xkb_compose_format format,
                          enum xkb_compose_compile_flags flags);

struct xkb_compose *
xkb_compose_new_from_buffer(struct xkb_context *context,
                            const char *buffer, size_t length,
                            const char *locale,
                            enum xkb_compose_format format,
                            enum xkb_compose_compile_flags flags);

struct xkb_compose *
xkb_compose_new_from_locale(struct xkb_context *ctx,
                            const char *locale,
                            enum xkb_compose_compile_flags flags);

struct xkb_compose *
xkb_compose_ref(struct xkb_compose *compose);

void
xkb_compose_unref(struct xkb_compose *compose);

enum xkb_compose_state_flags {
    XKB_COMPOSE_STATE_NO_FLAGS = 0
};

struct xkb_compose_state *
xkb_compose_state_new(struct xkb_compose *compose,
                      enum xkb_compose_state_flags flags);

struct xkb_compose_state *
xkb_compose_state_ref(struct xkb_compose_state *state);

struct xkb_compose *
xkb_compose_state_get_compose(struct xkb_compose_state *state);

void
xkb_compose_state_unref(struct xkb_compose_state *state);

void
xkb_compose_state_feed(struct xkb_compose_state *state,
                       xkb_keysym_t keysym);

void
xkb_compose_state_reset(struct xkb_compose_state *state);

enum xkb_compose_status {
    XKB_COMPOSE_NOTHING,
    XKB_COMPOSE_COMPOSING,
    XKB_COMPOSE_COMPOSED,
    XKB_COMPOSE_CANCELLED
};

enum xkb_compose_status
xkb_compose_state_get_status(struct xkb_compose_state *state);

int
xkb_compose_state_get_utf8(struct xkb_compose_state *state,
                           char *buffer, size_t size);

xkb_keysym_t
xkb_compose_state_get_one_sym(struct xkb_compose_state *state);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XKBCOMMON_COMPOSE_H */
