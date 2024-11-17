#include "postgres.h"
#include "optimizer/planner.h" /* For planner_hook and Query */
#include "executor/spi.h"
#include "nodes/parsenodes.h"
#include "utils/elog.h"

PG_MODULE_MAGIC;

/* Function declarations */
void _PG_init(void);
void _PG_fini(void);

static planner_hook_type prev_planner_hook = NULL;

static PlannedStmt *safe_delete_shield_planner(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams);

/* Module initialization */
void _PG_init(void)
{
    prev_planner_hook = planner_hook;
    planner_hook = safe_delete_shield_planner;
}

void _PG_fini(void)
{
    planner_hook = prev_planner_hook;
}

/* Planner hook to enforce WHERE clause on DELETE */
static PlannedStmt *safe_delete_shield_planner(Query *parse, const char *query_string, int cursorOptions, ParamListInfo boundParams)
{
    /* Intercept DELETE statements */
    if (parse->commandType == CMD_DELETE)
    {
        /* Check if the WHERE clause is missing */
        if (parse->jointree->quals == NULL)
        {
            ereport(ERROR,
                    (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
                     errmsg("Deleting the entire table is not allowed in this database. Please use a WHERE clause.")));
        }
    }

    /* Call the previous planner hook or the standard planner */
    if (prev_planner_hook)
        return prev_planner_hook(parse, query_string, cursorOptions, boundParams);
    else
        return standard_planner(parse, query_string, cursorOptions, boundParams);
}
