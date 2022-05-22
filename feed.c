/* SPDX-License_identifier: AGPL-3.0-only */

#include "feed.h"

#include <libxml/parser.h>

/* Custom character type... fine, whatever. */
#define x (const xmlChar *)
#define g (char *)

#define assert(cond) \
    if (!(cond)) {                                                      \
        printf("assertion failure: feed.c line %d\n", __LINE__);        \
        exit(1);                                                        \
    }

struct feed_context {
    xmlDoc *doc;
    xmlNode *cur;
};

void unload_feed(feed_context *ctx) {
    if (!ctx) {
        return;
    }
    xmlFreeDoc(ctx->doc);
}

void feed_free(void *p) {
    xmlFree(p);
}

feed_context *load_feed(buf *raw_xml, char **title_out) {
    feed_context *ctx;
    xmlDoc *doc;
    xmlNode *cur;
    char *buffer;

    *title_out = NULL;

    buffer = destruct_buf(&raw_xml);
    doc = xmlParseDoc(x(buffer));
    assert(doc);

    cur = xmlDocGetRootElement(doc);
    assert(cur);
    assert(!xmlStrcmp(cur->name, x"rss"));

    for (cur = cur->children; cur != NULL; cur = cur->next) {
        if (!xmlStrcmp(cur->name, x"channel")) {
            break;
        }
    }

    for (xmlNode *t = cur->children; t != NULL; t = t->next) {
        if (!xmlStrcmp(t->name, x"title")) {
            *title_out = g(xmlNodeListGetString(doc, t->xmlChildrenNode, 1));
            break;
        }
    }

    ctx = malloc(sizeof(*ctx));
    assert(ctx);
    ctx->doc = doc;
    ctx->cur = cur->last; /* Walking backward in a moment. */
    return ctx;
}

bool next_item(feed_context *ctx, char **title_out, char **guid_out,
               char **url_out) {
    xmlChar *title = NULL, *guid = NULL, *url = NULL;

    *title_out = *guid_out = *url_out = NULL;

    if (ctx->cur == NULL) {
        return false;
    }

    do {
        ctx->cur = ctx->cur->prev;
        if (ctx->cur == NULL) {
            return false;
        }
    } while (xmlStrcmp(ctx->cur->name, x"item"));
    if (ctx->cur == NULL) {
        return false;
    }

    for (xmlNode *c = ctx->cur->children; c != NULL; c = c->next) {
        if (title == NULL && !xmlStrcmp(c->name, x"title")) {
            title = xmlNodeListGetString(ctx->doc, c->children, 1);
        } else if (guid == NULL && !xmlStrcmp(c->name, x"guid")) {
            guid = xmlNodeListGetString(ctx->doc, c->children, 1);
        } else if (url == NULL && !xmlStrcmp(c->name, x"enclosure")) {
            url = xmlGetProp(c, x"url");
        }
    }

    if (url == NULL) {
        /* Uh... skip this one and try again, I guess. */
        xmlFree(title);
        xmlFree(guid);
        return next_item(ctx, title_out, guid_out, url_out);
    }
    if (title == NULL) {
        title = xmlStrdup(url);
        assert(title);
    }
    if (guid == NULL) {
        /* A whole bunch of feeds do this anyway... */
        guid = xmlStrdup(url);
        assert(guid);
    }

    *title_out = g(title);
    *url_out = g(url);
    *guid_out = g(guid);
    return true;
}

/* Local variables: */
/* c-basic-offset: 4 */
/* indent-tabs-mode: nil */
/* End: */
