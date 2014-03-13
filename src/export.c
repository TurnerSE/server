#include <util/bool.h>
#include <util/base36.h>
#include <platform.h>

#include "export.h"

#include <kernel/types.h>
#include <kernel/region.h>
#include <kernel/faction.h>
#include <kernel/terrain.h>
#include <stream.h>
#include "cJSON.h"

#include <assert.h>

int export_json(stream * out, unsigned int flags) {
    cJSON *json, *root = cJSON_CreateObject();
    assert(out && out->api);
    if (flags & EXPORT_REGIONS) {
        region * r;
        cJSON_AddItemToObject(root, "regions", json = cJSON_CreateObject());
        for (r = regions; r; r = r->next) {
            char id[32];
            cJSON *data;
            snprintf(id, sizeof(id), "%u", r->uid);
            cJSON_AddItemToObject(json, id, data = cJSON_CreateObject());
            cJSON_AddNumberToObject(data, "x", r->x);
            cJSON_AddNumberToObject(data, "y", r->y);
            cJSON_AddStringToObject(data, "type", r->terrain->_name);
        }
    }
    if (flags & EXPORT_FACTIONS) {
        faction *f;
        cJSON_AddItemToObject(root, "factions", json = cJSON_CreateObject());
        for (f = factions; f; f = f->next) {
            cJSON *data;
            cJSON_AddItemToObject(json, itoa36(f->no), data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "name", f->name);
            cJSON_AddStringToObject(data, "email", f->email);
            cJSON_AddNumberToObject(data, "score", f->score);
        }
    }
    if (flags) {
        char *tok, *output;
        output = cJSON_Print(json);
        tok = strtok(output, "\n\r");
        while (tok) {
            if (tok[0]) {
                out->api->writeln(out->handle, tok);
            }
            tok = strtok(NULL, "\n\r");
        }
        free(output);
    }
    cJSON_Delete(root);
    return 0;
}
