#include <bson.h>
#include <mongoc.h>

int mongodb_register(mongoc_database_t *, char *, char *, bson_error_t *);

int mongodb_login(mongoc_database_t *, const char *, const char *, char [25], bson_error_t *);

int mongodb_unregister(mongoc_database_t *, const char *, bson_error_t *);

int mongodb_get_user(mongoc_database_t *, const char *, void **, size_t *, bson_error_t *);

int mongodb_update_user(mongoc_database_t *, const char *, const char *, bson_error_t *);

int mongodb_get_keys(mongoc_database_t *, const char *, void **, size_t *, bson_error_t *);

int mongodb_replace_keys(mongoc_database_t *, const char *, const char *, bson_error_t *);

int mongodb_get_word(mongoc_database_t *db, const char *oid_str, const char **word_json, size_t *length, bson_error_t *error);

int mongodb_create_index(mongoc_database_t *db, bson_error_t *error);
