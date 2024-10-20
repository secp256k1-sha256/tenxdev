#include "postgres.h"
#include "commands/defrem.h"
#include "commands/event_trigger.h"
#include "nodes/makefuncs.h"
#include "nodes/parsenodes.h"
#include "tcop/utility.h"
#include "utils/elog.h"
#include "catalog/namespace.h"
#include "catalog/pg_class.h"
#include "executor/spi.h"
#include "utils/builtins.h"  // Include this for CStringGetTextDatum

PG_MODULE_MAGIC;

void _PG_init(void);
void _PG_fini(void);

static ProcessUtility_hook_type prev_ProcessUtility = NULL;

static bool column_has_index(char *tabname, char *colname);
static void fkhunter_ProcessUtility(PlannedStmt *pstmt, const char *queryString,
                                   bool readOnlyTree,
                                   ProcessUtilityContext context, ParamListInfo params,
                                   QueryEnvironment *queryEnv, DestReceiver *dest,
                                   QueryCompletion *qc);

static void fkhunter_ProcessUtility(PlannedStmt *pstmt, const char *queryString,
                                   bool readOnlyTree,
                                   ProcessUtilityContext context,
                                   ParamListInfo params, QueryEnvironment *queryEnv,
                                   DestReceiver *dest, QueryCompletion *qc) 
{
    Node       *parsetree = pstmt->utilityStmt;

    if (IsA(parsetree, AlterTableStmt) || IsA(parsetree, CreateStmt))
    {
        ListCell   *cell;

        if (IsA(parsetree, AlterTableStmt))
        {
            AlterTableStmt *atstmt = (AlterTableStmt *)parsetree;
            foreach(cell, atstmt->cmds)
            {
                AlterTableCmd *cmd = (AlterTableCmd *) lfirst(cell);
                if (cmd->subtype == AT_AddConstraint && IsA(cmd->def, Constraint))
                {
                    Constraint *constraint = (Constraint *) cmd->def;
                    if (constraint->contype == CONSTR_FOREIGN)
                    {
                        char *fk_table = atstmt->relation->relname;
                        char *fk_col = strVal(linitial(constraint->fk_attrs));
                        if (!column_has_index(fk_table, fk_col))
                        {
                            elog(ERROR, "Foreign Key creation on unindexed column is not allowed.");
                        }
                    }
                }
            }
        }
        else if (IsA(parsetree, CreateStmt))
        {
            CreateStmt *cstmt = (CreateStmt *) parsetree;
            foreach(cell, cstmt->tableElts)
            {
                if (IsA(lfirst(cell), Constraint))
                {
                    Constraint *constraint = (Constraint *) lfirst(cell);
                    if (constraint->contype == CONSTR_FOREIGN)
                    {
                        elog(ERROR, "Foreign Key creation is not allowed.");
                    }
                }
            }
        }
    }

    if (prev_ProcessUtility)
    {
        prev_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, qc);
    }
    else
    {
        standard_ProcessUtility(pstmt, queryString, readOnlyTree, context, params, queryEnv, dest, qc);
    }
}

static bool column_has_index(char *tabname, char *colname)
{
    bool result = false;
    const char *query = 
        "SELECT EXISTS ("
        "   SELECT 1 "
        "   FROM pg_class t "
        "   JOIN pg_index ix ON t.oid = ix.indrelid "
        "   JOIN pg_attribute a ON a.attrelid = t.oid AND a.attnum = ANY(ix.indkey) "
        "   WHERE t.relkind = 'r' AND t.relname = $1 AND a.attname = $2"
        ")";
    
    Oid argtypes[2] = {TEXTOID, TEXTOID};  // Moved declarations here
    Datum args[2];
    int ret;

    SPI_connect();
    
    args[0] = CStringGetTextDatum(tabname);
    args[1] = CStringGetTextDatum(colname);
    ret = SPI_execute_with_args(query, 2, argtypes, args, NULL, true, 0);

    if (ret == SPI_OK_SELECT && SPI_processed > 0)
    {
        bool isnull;
        Datum exists = SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, &isnull);
        result = DatumGetBool(exists);
    }

    SPI_finish();
    return result;
}

void _PG_init(void)
{
    prev_ProcessUtility = ProcessUtility_hook;
    ProcessUtility_hook = fkhunter_ProcessUtility;
}

void _PG_fini(void)
{
    ProcessUtility_hook = prev_ProcessUtility;
}
