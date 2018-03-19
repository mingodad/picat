/********************************************************************
 *   File   : picat_sqlite3.c
 *   Author : Domingo Alvarez Duarte Copyright (C) 1994-2018
 *   https://github.com/mingodad/picat

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 ********************************************************************/

#ifdef WITH_SQLITE3

#include "picat.h"
#include "extern_decl.h"
#include "sqlite3.h"

typedef struct picat_sqlite3_st {
    sqlite3 *db;
};

typedef struct picat_sqlite3_stmt_st {
    sqlite3_stmt *stmt;
};

static sqlite3 *db = NULL;
static BPLONG dbIdx = 0;

static int checkDbParam(int pidx, int nparams)
{
    BPLONG dbId;
    dbId = bp_get_integer(ARG(pidx,nparams));
    if(!dbIdx || (dbId != dbIdx))
    {
        exception = illegal_arguments;
        return BP_ERROR;
    }
    return PICAT_TRUE;
}

int picat_sqlite3_open()
{
    TERM a2, result;
    const char *db_fname;

    if(!db)
    {
        db_fname = (const char *)bp_get_atom_name(ARG(1,2));
        a2 = picat_get_call_arg(2, 2);
        if(db_fname)
        {
            int rc = sqlite3_open(db_fname, &db);
            if(rc == SQLITE_OK)
            {
                dbIdx = 1573;
                result = picat_build_integer(dbIdx);
                return picat_unify(a2, result);
            }
            else exception = illegal_arguments;
        }
        else exception = illegal_arguments;
    }
    else exception = out_of_range;

    return BP_ERROR;
}

int picat_sqlite3_exec_dml()
{
    TERM a3, result;

    int rc = checkDbParam(1,3);
    if(rc == PICAT_TRUE)
    {
        const char *sql, *zTail;
        sqlite3_stmt *stmt;
        sql = (const char *)bp_get_atom_name(ARG(2,3));
        a3 = picat_get_call_arg(3, 3);
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &zTail);
        if(rc == SQLITE_OK)
        {
            rc = sqlite3_step(stmt);
            if(rc == SQLITE_DONE)
            {
                result = picat_build_integer(1);
                picat_unify(a3, result);
                rc = PICAT_TRUE;
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            exception = run_time_error;
            return BP_ERROR;
        }
    }
    return rc;
}

int picat_sqlite3_close()
{
    int rc = checkDbParam(1,1);
    if(rc == PICAT_TRUE)
    {
        sqlite3_close(db);
        dbIdx = 0;
        db = NULL;
    }
    return rc;
}

#endif // WITH_SQLITE3
