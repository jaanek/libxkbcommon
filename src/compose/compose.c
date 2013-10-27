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

#include "utils.h"
#include "compose.h"
#include "parser.h"
#include "paths.h"

static struct xkb_compose *
xkb_compose_new(struct xkb_context *ctx,
                const char *locale,
                enum xkb_compose_format format,
                enum xkb_compose_compile_flags flags)
{
    char *resolved_locale;
    struct xkb_compose *compose;
    struct node root;

    resolved_locale = resolve_locale(locale);
    if (!resolved_locale)
        return NULL;

    compose = calloc(1, sizeof(*compose));
    if (!compose) {
        free(resolved_locale);
        return NULL;
    }

    compose->refcnt = 1;
    compose->ctx = xkb_context_ref(ctx);

    compose->locale = resolved_locale;
    compose->format = format;
    compose->flags = flags;

    darray_init(compose->tree);
    darray_init(compose->utf8);

    root.keysym = XKB_KEY_NoSymbol;
    root.next = 0;
    root.successor = 0;
    root.utf8 = 0;
    root.ks = XKB_KEY_NoSymbol;
    darray_append(compose->tree, root);

    darray_append(compose->utf8, '\0');

    return compose;
}

XKB_EXPORT struct xkb_compose *
xkb_compose_ref(struct xkb_compose *compose)
{
    compose->refcnt++;
    return compose;
}

XKB_EXPORT void
xkb_compose_unref(struct xkb_compose *compose)
{
    if (!compose || --compose->refcnt > 0)
        return;
    free(compose->locale);
    darray_free(compose->tree);
    darray_free(compose->utf8);
    xkb_context_unref(compose->ctx);
    free(compose);
}

XKB_EXPORT struct xkb_compose *
xkb_compose_new_from_file(struct xkb_context *ctx,
                          FILE *file,
                          const char *locale,
                          enum xkb_compose_format format,
                          enum xkb_compose_compile_flags flags)
{
    struct xkb_compose *compose;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    if (format != XKB_COMPOSE_FORMAT_TEXT_V1) {
        log_err_func(ctx, "unsupported compose format: %d\n", format);
        return NULL;
    }

    compose = xkb_compose_new(ctx, locale, format, flags);
    if (!compose)
        return NULL;

    ok = parse_file(compose, file, "(unknown file)");
    if (!ok) {
        xkb_compose_unref(compose);
        return NULL;
    }

    return compose;
}

XKB_EXPORT struct xkb_compose *
xkb_compose_new_from_buffer(struct xkb_context *ctx,
                            const char *buffer, size_t length,
                            const char *locale,
                            enum xkb_compose_format format,
                            enum xkb_compose_compile_flags flags)
{
    struct xkb_compose *compose;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    if (format != XKB_COMPOSE_FORMAT_TEXT_V1) {
        log_err_func(ctx, "unsupported compose format: %d\n", format);
        return NULL;
    }

    compose = xkb_compose_new(ctx, locale, format, flags);
    if (!compose)
        return NULL;

    ok = parse_string(compose, buffer, length, "(input string)");
    if (!ok) {
        xkb_compose_unref(compose);
        return NULL;
    }

    return compose;
}

XKB_EXPORT struct xkb_compose *
xkb_compose_new_from_locale(struct xkb_context *ctx,
                            const char *locale,
                            enum xkb_compose_compile_flags flags)
{
    struct xkb_compose *compose;
    char *path = NULL;
    const char *cpath;
    FILE *file;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    compose = xkb_compose_new(ctx, locale, XKB_COMPOSE_FORMAT_TEXT_V1, flags);
    if (!compose)
        return NULL;

    cpath = get_xcomposefile_path();
    if (cpath) {
        file = fopen(cpath, "r");
        if (file)
            goto found_path;
    }

    cpath = path = get_home_xcompose_file_path();
    if (path) {
        file = fopen(path, "r");
        if (file)
            goto found_path;
    }
    free(path);
    path = NULL;

    cpath = path = get_locale_compose_file_path(compose->locale);
    if (path) {
        file = fopen(path, "r");
        if (file)
            goto found_path;
    }
    free(path);
    path = NULL;

    log_err(ctx, "couldn't find a Compose file for locale \"%s\"\n", locale);
    xkb_compose_unref(compose);
    return NULL;

found_path:
    ok = parse_file(compose, file, cpath);
    fclose(file);
    if (!ok) {
        xkb_compose_unref(compose);
        return NULL;
    }

    log_dbg(ctx, "created compose from locale %s with path %s\n",
            compose->locale, path);

    free(path);
    return compose;
}
