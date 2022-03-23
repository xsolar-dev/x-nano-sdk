#ifndef NNG_MQTT_QOS_DB_H
#define NNG_MQTT_QOS_DB_H

#include "core/nng_impl.h"
#include "nng/nng.h"
#include "supplemental/sqlite/sqlite3.h"

/**
 pipe_client_table
----------------------------
|  id  | pipe_id |client_id|
----------------------------
|      |         |         |
----------------------------
**/

/**
 msg_table
---------------------
|    id   |   data  |
---------------------
|         |         |
---------------------
**/

/**
 main_table
-----------------------
| p_id | msg_id | qos |
-----------------------
|      |        |     |
-----------------------
**/

#define MQTT_DB_GET_QOS_BITS(msg) ((size_t) (msg) &0x03)
#define MQTT_DB_PACKED_MSG_QOS(msg, qos) \
	((nni_msg *) ((size_t) (msg) | ((qos) &0x03)))

#define MQTT_DB_GET_MSG_POINTER(msg) ((nni_msg *) ((size_t) (msg) & (~0x03)))

NNG_DECL void nni_mqtt_qos_db_init(sqlite3 **);
NNG_DECL void nni_mqtt_qos_db_close(sqlite3 *);
NNG_DECL void nni_mqtt_qos_db_set(sqlite3 *, uint32_t, nni_msg *);
NNG_DECL nni_msg *nni_mqtt_qos_db_get(sqlite3 *, uint32_t);
NNG_DECL void     nni_mqtt_qos_db_remove(sqlite3 *, uint32_t);
NNG_DECL void nni_mqtt_qos_db_foreach(sqlite3 *db, void (*cb)(void *, void *));
NNG_DECL void nni_mqtt_qos_db_set_pipe(sqlite3 *, uint32_t, const char *);
NNG_DECL void nni_mqtt_qos_db_insert_pipe(sqlite3 *, uint32_t, const char *);
NNG_DECL
void nni_mqtt_qos_db_remove_pipe(sqlite3 *, uint32_t);
NNG_DECL
void nni_mqtt_qos_db_update_pipe_by_clientid(
    sqlite3 *, uint32_t, const char *);
NNG_DECL void nni_mqtt_qos_db_update_all_pipe(sqlite3 *, uint32_t);
NNG_DECL void nni_mqtt_qos_db_check_remove_msg(sqlite3 *, nni_msg *);

#endif