#include <stddef.h>
#include <bson.h>
#include <mongoc.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
var keys = {"email": 1};
var options = {"unique": true};
db.users.createIndex(keys, options);
*/
int mongodb_create_index(mongoc_database_t *db, bson_error_t *error)
{
    bson_t *keys = bson_new();
    if (!bson_append_int32(keys, "email", -1, 1)) {
        error->code = 1;
        sprintf(error->message, "bson append failed");
        goto error_append;
    }

    mongoc_index_opt_t options;
    mongoc_index_opt_init(&options);
    options.unique = 1;

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    if (!mongoc_collection_create_index(users, keys, &options, error)) {
        goto error_create;
    }

error_create:
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(keys);
    return error->code;
}

int mongodb_register(mongoc_database_t *db, const char *user_json, const char *keys_json, bson_error_t *error)
{
    bson_t *user = NULL, *keys = NULL;
    if ((user = bson_new_from_json((uint8_t *) user_json, -1, error)) == NULL ||
        (keys = bson_new_from_json((uint8_t *) keys_json, -1, error)) == NULL
    ) {
        goto error_json;
    }

    if (!bson_append_document(user, "keys", -1, keys)) {
       goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    if (!mongoc_collection_insert(users, MONGOC_INSERT_NONE, user, NULL, error)) {
        goto error_insert;
    }

error_insert:
    mongoc_collection_destroy(users);
error_append:
error_json:
    if (keys != NULL) bson_destroy(keys);
    if (user != NULL) bson_destroy(user);
    return error->code;
}

int mongodb_login(mongoc_database_t *db, const char *email, const char *password, char oid_str[25], bson_error_t *error)
{
    bson_t *query = bson_new(), filter, projection;
    if (!bson_append_document_begin(query, "filter", -1, &filter) ||
        !bson_append_utf8(&filter, "email", -1, email, -1) ||
        !bson_append_utf8(&filter, "password", -1, password, -1) ||
        !bson_append_document_end(query, &filter) ||
        !bson_append_document_begin(query, "projection", -1, &projection) ||
        !bson_append_int32(&projection, "_id", -1, 1) ||
        !bson_append_document_end(query, &projection) ||
        !bson_append_int32(query, "limit", -1, 1)
    ) {
        sprintf(error->message, "bson append failed");
        error->code = 1;
        goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    mongoc_cursor_t *cursor = mongoc_collection_find(users, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    assert(cursor != NULL);
    const bson_t *user = NULL;
    if (!mongoc_cursor_next(cursor, &user)) {
        mongoc_cursor_error(cursor, error);
        goto error_find;
    }

    bson_iter_t iter;
    if (bson_iter_init_find(&iter, user, "_id") && bson_iter_type(&iter) == BSON_TYPE_OID) {
        const bson_oid_t *oid = bson_iter_oid(&iter);
        bson_oid_to_string(oid, oid_str);
    } else {
        oid_str[0] = 0;
    }

error_find:
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(&projection);
    bson_destroy(&filter);
    bson_destroy(query);
    return error->code;
}

int mongodb_unregister(mongoc_database_t *db, const char *oid_str, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);
    bson_t *selector = bson_new();
    if (!bson_append_oid(selector, "_id", -1, &oid)) {
        goto error_append;
    }
    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    if (!mongoc_collection_remove(users, MONGOC_REMOVE_SINGLE_REMOVE, selector, NULL, error)) {
        goto error_remove;
    }

error_remove:
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(selector);
    return error->code;
}

int mongodb_get_user(mongoc_database_t *db, const char *oid_str, void **user_json, size_t *length, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);
    bson_t *query = bson_new(), filter, projection;
    if (!bson_append_document_begin(query, "filter", -1, &filter) ||
        !bson_append_oid(&filter, "_id", -1, &oid) ||
        !bson_append_document_end(query, &filter) ||
        !bson_append_document_begin(query, "projection", -1, &projection) ||
        !bson_append_int32(&projection, "keys", -1, 0) ||
        !bson_append_int32(&projection, "words", -1, 0) ||
        !bson_append_document_end(query, &projection) ||
        !bson_append_int32(query, "limit", -1, 1)
    ) {
        error->code = 1;
        sprintf(error->message, "bson append failed");
        goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    mongoc_cursor_t *cursor = mongoc_collection_find(users, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    const bson_t *user = NULL;
    if (!mongoc_cursor_next(cursor, &user)) {
        mongoc_cursor_error(cursor, error);
        goto error_find;
    }
    if ((*user_json = bson_as_json(user, length)) == NULL) {
        error->code = 1;
        sprintf(error->message, "bson to json convertion failed");
    }

error_find:
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(query);
    bson_destroy(&filter);
    bson_destroy(&projection);
    return error->code;
}

int mongodb_update_user(mongoc_database_t *db, const char *oid_str, const char *user_json, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);

    bson_t *user;
    if ((user = bson_new_from_json((uint8_t *) user_json, -1, error)) == NULL) {
        goto error_json;
    }

    bson_t *selector = bson_new(), *update = bson_new();
    if (!bson_append_oid(selector, "_id", -1, &oid) ||
        !bson_append_document(update, "$set", -1, user)
    ) {
        error->code = 1;
        sprintf(error->message, "bson append failed");
        goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    if (!mongoc_collection_update(users, MONGOC_UPDATE_NONE, selector, update, NULL, error)) {
        goto error_update;
    }
error_update:
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(selector);
    bson_destroy(update);
error_json:
    return error->code;
}

int mongodb_get_keys(mongoc_database_t *db, const char *oid_str, void **keys_json, size_t *length, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);

    bson_t *query = bson_new(), filter, projection;
    if (!bson_append_document_begin(query, "filter", -1, &filter) ||
        !bson_append_oid(&filter, "_id", -1, &oid) ||
        !bson_append_document_end(query, &filter) ||
        !bson_append_document_begin(query, "projection", -1, &projection) ||
        !bson_append_int32(&projection, "_id", -1, -1) ||
        !bson_append_int32(&projection, "keys", -1, 1) ||
        !bson_append_document_end(query, &projection) ||
        !bson_append_int32(query, "limit", -1, 1)
    ) {
        error->code = 1;
        sprintf(error->message, "bson append failed");
        goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    mongoc_cursor_t *cursor = mongoc_collection_find(users, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    const bson_t *user;
    if (!mongoc_cursor_next(cursor, &user)) {
        goto error_find;
    }

    bson_iter_t iter;
    const uint8_t *data;
    uint32_t data_length;
    bson_t *keys;
    if (bson_iter_init_find(&iter, user, "keys") && bson_iter_type(&iter) == BSON_TYPE_DOCUMENT) {
        bson_iter_document(&iter, &data_length, &data);
        assert((keys = bson_new_from_data(data, data_length)) != NULL);
        assert((*keys_json = bson_as_json(keys, length)) != NULL);
    } else {
        error->code = 1;
        sprintf(error->message, "keys not found");
        goto error_iter;
    }

    bson_destroy(keys);
error_iter:
error_find:
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(&filter);
    bson_destroy(query);
    return error->code;
}

int mongodb_replace_keys(mongoc_database_t *db, const char *oid_str, const char *keys_json, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);

    bson_t *keys;
    if ((keys = bson_new_from_json((uint8_t *) keys_json, -1, error)) == NULL) {
        goto error_json;
    }

    bson_t *selector = bson_new(), *update = bson_new(), set;
    if (!bson_append_oid(selector, "_id", -1, &oid) ||
        !bson_append_document_begin(update, "$set", -1, &set) ||
        !bson_append_document(&set, "keys", -1, keys) ||
        !bson_append_document_end(update, &set)
    ) {
        error->code = 1;
        sprintf(error->message, "keys not found");
        goto error_append;
    }

    mongoc_collection_t *users = mongoc_database_get_collection(db, "users");
    if (!mongoc_collection_update(users, MONGOC_UPDATE_NONE, selector, update, NULL, error)) {
        goto error_update;
    }

error_update:
    mongoc_collection_destroy(users);
error_append:
    bson_destroy(&set);
    bson_destroy(update);
    bson_destroy(selector);
    bson_destroy(keys);
error_json:
    return error->code;
}

/*
var query = {"_id": ObjectID(oid_str)};
var projection = {"_id": 1, "spelling": 1, "notation": 1, "sound": 1};
db.words.find(query, projection);
*/
int mongodb_get_word(mongoc_database_t *db, const char *oid_str, const char **word_json, size_t *length, bson_error_t *error)
{
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, oid_str);

    bson_t *query = bson_new(), *projection = bson_new();
    if (!bson_append_oid(query, "_id", -1, &oid) ||
        !bson_append_int32(projection, "_id", -1, 1) ||
        !bson_append_int32(projection, "spelling", -1, 1) ||
        !bson_append_int32(projection, "notation", -1, 1) ||
        !bson_append_int32(projection, "sound", -1, 1)
    ) {
        error->code = 1;
        sprintf(error->message, "append bson failed");
        goto error_append;
    }

    mongoc_collection_t *words = mongoc_database_get_collection(db, "words");
    mongoc_cursor_t *cursor = mongoc_collection_find(words, MONGOC_QUERY_NONE, 0, 1, 0, query, projection, NULL);
    const bson_t *word;
    if (!mongoc_cursor_next(cursor, &word)) {
       mongoc_cursor_error(cursor, error);
       goto error_find;
    }

    if ((*word_json = bson_as_json(word, length)) == NULL) {
        error->code = 1;
        sprintf(error->message, "bson to json failed");
        goto error_bson_to_json;
    }

error_bson_to_json:
error_find:
    mongoc_collection_destroy(words);
    mongoc_cursor_destroy(cursor);
error_append:
    bson_destroy(query);
    bson_destroy(projection);
    return error->code;
}

/*
db.words.insert(word_json);
*/
//int mongodb_put_word(mongoc_database_t *db, bson_t *word_json, bson_error_t *error)
//{
//}
