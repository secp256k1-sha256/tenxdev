#include "postgres.h"
#include "tcop/utility.h"
#include "commands/event_trigger.h"
#include "commands/defrem.h"
#include "utils/elog.h"
#include "parser/parse_node.h"
#include "utils/lsyscache.h"
#include <ctype.h>
#include <string.h>

PG_MODULE_MAGIC;

/* Function declarations */
void _PG_init(void);
void _PG_fini(void);

static ProcessUtility_hook_type prev_ProcessUtility = NULL;

static void enforce_concurrent_indexes(
    PlannedStmt *pstmt,
    const char *queryString,
    bool readOnlyTree,
    ProcessUtilityContext context,
    ParamListInfo params,
    QueryEnvironment *queryEnv,
    DestReceiver *dest,
    QueryCompletion *completion);

static bool contains_concurrently(const char *queryString);

/* Module initialization */
void _PG_init(void)
{
    prev_ProcessUtility = ProcessUtility_hook;
    ProcessUtility_hook = enforce_concurrent_indexes;
}

void _PG_fini(void)
{
    ProcessUtility_hook = prev_ProcessUtility;
}

/* Convert a string to uppercase */
static void to_uppercase(char *dest, const char *src)
{
    while (*src)
    {
        *dest = toupper((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

/* Utility function to check if a string contains "CONCURRENTLY" (case insensitive) */
static bool contains_concurrently(const char *queryString)
{
    char *uppercaseQuery;
    const char *needle = "CONCURRENTLY";
    bool found;

    uppercaseQuery = palloc(strlen(queryString) + 1);
    to_uppercase(uppercaseQuery, queryString);

    found = strstr(uppercaseQuery, needle) != NULL;

    pfree(uppercaseQuery);
    return found;
}

/* Main utility hook */
static void enforce_concurrent_indexes(
    PlannedStmt *pstmt,
    const char *queryString,
    bool readOnlyTree,
    ProcessUtilityContext context,
    ParamListInfo params,
    QueryEnvironment *queryEnv,
    DestReceiver *dest,
    QueryCompletion *completion)
{
    Node *parsetree = pstmt->utilityStmt;

    /* Handle CREATE INDEX statements */
    if (nodeTag(parsetree) == T_IndexStmt)
    {
        IndexStmt *stmt = (IndexStmt *)parsetree;

        if (!stmt->concurrent)
        {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("Index creation must use the CONCURRENTLY keyword to ensure online index building.")));
        }
    }
    /* Handle REINDEX statements */
    else if (nodeTag(parsetree) == T_ReindexStmt)
    {
        /* REINDEX does not have a `concurrent` flag, so parse the query string */
        if (!contains_concurrently(queryString))
        {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("REINDEX operations must use the CONCURRENTLY keyword for online index rebuilding.")));
        }
    }

    /* Call the previous hook or the standard process utility function */
    if (prev_ProcessUtility)
        prev_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
    else
        standard_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
}
