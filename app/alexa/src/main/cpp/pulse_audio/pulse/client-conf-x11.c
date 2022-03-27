#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <xcb/xcb.h>
#include "xmalloc.h"
#include <pulse_audio/pulsecore/i18n.h>
#include <pulse_audio/pulsecore/x11prop.h>
#include <pulse_audio/pulsecore/log.h>
#include <pulse_audio/pulsecore/core-util.h>
#include <pulse_audio/pulsecore/macro.h>

#include "client-conf-x11.h"

int pa_client_conf_from_x11(pa_client_conf *c) {
    const char *dname;
    xcb_connection_t *xcb = NULL;
    int ret = -1, screen = 0;
    char t[1024];

    pa_assert(c);

    /* Local connections will have configuration and X root window
     * properties match 1:1, these paths are only strictly necessary
     * for remote clients, so check for SSH_CONNECTION to make sure
     * this is a remote session with X forwarding.
     */
    if (!getenv("SSH_CONNECTION"))
        goto finish;

    if (!(dname = getenv("DISPLAY")))
        goto finish;

    if (*dname == 0)
        goto finish;

    if (!(xcb = xcb_connect(dname, NULL))) {
        pa_log(_("xcb_connect() failed"));
        goto finish;
    }

    if (xcb_connection_has_error(xcb)) {
        pa_log(_("xcb_connection_has_error() returned true"));
        goto finish;
    }

    if (pa_x11_get_prop(xcb, screen, "PULSE_SERVER", t, sizeof(t))) {
        bool disable_autospawn = true;

        pa_xfree(c->default_server);
        c->default_server = pa_xstrdup(t);

        if (pa_x11_get_prop(xcb, screen, "PULSE_SESSION_ID", t, sizeof(t))) {
            char *id;

            if ((id = pa_session_id())) {
                if (pa_streq(t, id))
                    disable_autospawn = false;
                pa_xfree(id);
            }
        }

        if (disable_autospawn)
            c->autospawn = false;
    }

    if (pa_x11_get_prop(xcb, screen, "PULSE_SINK", t, sizeof(t))) {
        pa_xfree(c->default_sink);
        c->default_sink = pa_xstrdup(t);
    }

    if (pa_x11_get_prop(xcb, screen, "PULSE_SOURCE", t, sizeof(t))) {
        pa_xfree(c->default_source);
        c->default_source = pa_xstrdup(t);
    }

    if (pa_x11_get_prop(xcb, screen, "PULSE_COOKIE", t, sizeof(t))) {
        if (pa_parsehex(t, c->cookie_from_x11, sizeof(c->cookie_from_x11)) != sizeof(c->cookie_from_x11)) {
            pa_log(_("Failed to parse cookie data"));
            goto finish;
        }

        c->cookie_from_x11_valid = true;
    }

    ret = 0;

finish:
    if (xcb)
        xcb_disconnect(xcb);

    return ret;

}
