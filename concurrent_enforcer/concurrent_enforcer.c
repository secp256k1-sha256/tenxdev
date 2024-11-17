#include "postgres.h"
#include "tcop/utility.h"
#include "commands/event_trigger.h"
#include "commands/defrem.h"
#include "parser/parse_node.h"
#include "utils/elog.h"
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
static bool is_index_operation(PlannedStmt *pstmt);
static bool is_table_creation_context(ProcessUtilityContext context);

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

/* Utility function to check if the statement is for index creation or reindexing */
static bool is_index_operation(PlannedStmt *pstmt)
{
    if (pstmt->commandType == CMD_UTILITY)
    {
        Node *parsetree = pstmt->utilityStmt;

        /* Check for CREATE INDEX statements */
        if (IsA(parsetree, IndexStmt))
        {
            return true; /* Index creation */
        }

        /* Check for REINDEX statements */
        if (IsA(parsetree, ReindexStmt))
        {
            return true; /* Reindexing */
        }
    }
    return false;
}

/* Utility function to check if we are in the context of a CREATE TABLE statement */
static bool is_table_creation_context(ProcessUtilityContext context)
{
    /* ProcessUtilityContext indicates whether we're inside a table creation process */
    return context == PROCESS_UTILITY_SUBCOMMAND;
}

/* Enforce concurrent indexes */
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
    /* Skip enforcement if we're inside a table creation context */
    if (is_table_creation_context(context))
    {
        if (prev_ProcessUtility)
            prev_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
        else
            standard_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
        return;
    }

    /* Apply enforcement only for explicit index creation or reindexing */
    if (is_index_operation(pstmt))
    {
        /* Check for CONCURRENTLY in the query string */
        if (!contains_concurrently(queryString))
        {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("All index creation and reindexing operations must use the CONCURRENTLY keyword")));
        }
    }

    /* Pass control to the next ProcessUtility hook */
    if (prev_ProcessUtility)
        prev_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
    else
        standard_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, completion);
}
