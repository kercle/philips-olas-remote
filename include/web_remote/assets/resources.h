// DO NOT MODIFY THIS FILE
// This file is modified by `scripts/embed_web.py`

#pragma once

#define _WEB_SERVER_REGISTER_ROUTE(server, path, mimetype, data) \
    (server).on((path), HTTP_GET, []() { \
        (server).send_P(200, (mimetype), (data)); \
    });

#define WEB_SERVER_REGISTER_STATIC_ASSETS(server)
