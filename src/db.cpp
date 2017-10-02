#include "db.h"

#include "patch.h"

#include <my_global.h>
#include <mysql.h>

MYSQL *con = NULL;

bool DBInit(const char *username, const char *password, const char *db) {
    con = mysql_init(NULL);
    if (con == NULL) {
        printf(" ERROR | Unable to do preliminary mysql connection\n");
        printf("       | %s\n", mysql_error(con));
        return false;
    }

    if (mysql_real_connect(con, "localhost", username, password,
                           NULL, 0, NULL, 0) == NULL) {
        printf(" ERROR | Unable to connect to mysql server\n");
        printf("       | %s\n", mysql_error(con));
        return false;
    }

    if (mysql_select_db(con, db)) {
        printf(" ERROR | Unable to select database '%s'\n", db);
        printf("       | %s\n", mysql_error(con));
        return false;
    }

    return true;
}

const char *insert_statement = R"(
INSERT liveupdates SET
updateID=%d, updateName='%s', description='%s',
machoVersionMin=%d, machoVersionMax=%d,
buildNumberMin=%d, buildNumberMax=%d,
methodName='%s', objectID='%s', codeType='%s', code='%s';
)";

bool DBCleanLiveupdates() {
    if (mysql_query(con, "TRUNCATE TABLE liveupdates;")) {
        printf(" ERROR | Unable to truncate liveupdates\n");
        printf("       | %s\n", mysql_error(con));
        return false;
    }
    return true;
}

bool DBApplyPatch(Patch *p, int id) {
    char buffer[10000];
    char *codebuf = (char *)calloc((p->bytecode_size * 2) + 1, 1);
    mysql_real_escape_string(con, codebuf, p->bytecode, p->bytecode_size);
    snprintf(buffer, 10000, insert_statement,
             id, p->name, "Generated by evegen",
             1, 330,
             1, 500000,
             p->method_name, p->class_name, p->type, codebuf
             );
    if (mysql_query(con, buffer)) {
        printf(" ERROR | %s\n", mysql_error(con));
        return false;
    }
    return true;
}
