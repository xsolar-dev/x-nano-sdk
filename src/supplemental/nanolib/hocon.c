#include "nng/supplemental/nanolib/hocon.h"
#include "nng/supplemental/nanolib/cvector.h"
#include "stdbool.h"
#include "stdio.h"

static char *skip_whitespace(char *str)
{
    if (NULL == str) {
        return NULL;
    }

    // Skip white_space and tab with bounds check 
    while ('\0' != *str && (' ' == *str || '\t' == *str)) {
        str++;
    }

    return str;

}

static cJSON *path_expression_parse_core(cJSON *parent, cJSON *jso)
{
    char *str = jso->string;
    char *p = jso->string;
    char *p_a = jso->string + strlen(jso->string);
    jso = cJSON_GetObjectItem(parent, jso->string);

    char t[128] = { 0 };
    while (NULL != (p = strrchr(str, '.'))) {
        cJSON *jso_new = cJSON_CreateObject();
        // cJSON *jso_new = NULL;

        // a.b.c: {object}
        // c ==> create json object jso(c, jso)
        *p = '_';
        strncpy(t, p + 1, p_a - p);
        // cJSON_AddItemToObject(jso_new, t, jso);
        cJSON_AddItemToObject(jso_new, t, cJSON_Duplicate(jso, cJSON_True));
        memset(t, 0, 128);
        // jso_new = json(c, jso)
        // cJSON_Delete(jso);
        jso = jso_new;
        jso_new = NULL;
        p_a = --p;
    }

    strncpy(t, str, p_a - str + 1);
    cJSON_AddItemToObject(parent, t, jso);
    memset(t, 0, 128);
    cJSON_DeleteItemFromObject(parent, str);

    return parent;
}

// {"bridge.sqlite":{"enable":false,"disk_cache_size":102400,"mounted_file_path":"/tmp/","flush_mem_threshold":100,"resend_interval":5000}}
// {"bridge":{"sqlite":{"enable":false,"disk_cache_size":102400,"mounted_file_path":"/tmp/","flush_mem_threshold":100,"resend_interval":5000}}}

// level-order traversal
// find key bridge.sqlite 
// create object sqlite with object value
// insert object bridge with sqlite
// delete bridge.sqlite

static cJSON *path_expression_parse(cJSON *jso)
{
    cJSON *parent = jso;
    cJSON *child = jso->child;

    while (child) {
        if (child->child) {
            path_expression_parse(child);
        }
        if (NULL != child->string && NULL != strchr(child->string, '.')) {
            path_expression_parse_core(parent, child);
            child = parent->child;
        } else {
            child = child->next;
        }
    }

    return jso;
}

// if same level has same name, if they are not object 
// the back covers the front
// TODO FIXME memory leak
cJSON *deduplication_and_merging(cJSON *jso)
{
    cJSON *parent = jso;
    cJSON *child = jso->child;
    cJSON **table = NULL;


    while (child) {
        for (size_t i = 0; i < cvector_size(table); i++) {
            if (0 == strcmp(table[i]->string, child->string)) {
                if (table[i]->type == child->type && cJSON_Object == table[i]->type) {
                    // merging object
                    cJSON *next = child->child->next;
                    while (next) {
                        next = next->next;
                    }
                    next = table[i]->child;
                    table[i]->child = NULL;
                    cJSON_DeleteItemFromObject(parent, table[i]->string);
                    // cJSON_Delete(table[i]->child);
                    cvector_erase(table, 1);

                } else {
                    if (0 == i) {
                        parent->child = child;
                        // cJSON_Delete(table[i]);
                        cJSON_free(table[i]);
                        cvector_erase(table, i);
                        
                    } else {
                        cJSON *free = table[i-1]->next;
                        table[i-1]->next = table[i-1]->next->next;
                        cvector_erase(table, i);
                        cJSON_free(free);
                        // cJSON_Delete(free);
                    }
                }
            }
        }

        cvector_push_back(table, child);

        if (child->child) {
            deduplication_and_merging(child);
        }
        child = child->next;
    }
    cvector_free(table);
    return jso;
}


cJSON *hocon_str_to_json(char *str)
{
    if (NULL == str) {
        return NULL;
    }

    // If it's not an illegal json object return
    cJSON *jso = cJSON_Parse(str);
    if (cJSON_False == cJSON_IsInvalid(jso)) {
        return jso;
    }

    char *p = str;
    int index = 0;
    char new[strlen(str) * 2];
    memset(new, 0, sizeof(new));

    new[index++] = '{';
    new[index++] = '"';

    // Replace all '=' to ':' 
    // If there are no '=' before object, add ':'
    // If first non-blank character is not '{', push-front '{' and push push-back '}'
    // Replace key to \"key\"
    while ('\0' != *p && NULL != (p = skip_whitespace(p))) {
        while (' ' != *p && '\0' != *p) {

            if ('}' == new[index-1]) {
                new[index++] = ',';
                new[index++] = '"';
            }

            if ('{' == *p && ':' != new[index-1]) {
                new[index++] = '"';
                new[index++] = ':';
            }

            if ('=' == *p) {
                new[index++] = '"';
                new[index++] = ':';
            } else if (',' == *p || '{' == *p) {
                new[index++] = *p;
                // TODO FIXME unsafe
                if ('}' != *(p+1)) {
                    new[index++] = '"';
                }
            } else {
                new[index++] = *p;
            }

            // remove ,"}
            if ('}' == new[index-1] && '"' == new[index-2] && ',' == new[index-3]) {
                new[index-3] = new[index-1];
                new[index-2] = '\0';
                new[index-1] = '\0';
                index -= 2;
            }

            p++;

        }
    }

    new[index++] = '}';


    if ((jso = cJSON_Parse(new))) {
        if (cJSON_False != cJSON_IsInvalid(jso)) {
            jso = path_expression_parse(jso);
            return deduplication_and_merging(jso);
        }
    }

    return NULL;

}