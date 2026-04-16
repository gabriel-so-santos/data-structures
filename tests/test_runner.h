/**
 * @file    test_runner.h
 * @brief   test functions prototypes
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 */

#ifndef LIBDS_TEST_RUNNER_H
#define LIBDS_TEST_RUNNER_H

#include "libds/core.h"
#include "libds/impl/nodechain.h"
#include "libds/listdef.h"
#include "libds/stackdef.h"
#include "libds/queuedef.h"



static ds_destructor_fn null_destroy = NULL;
static ds_copier_fn null_copy = NULL;

void run_nodechain_tests(void);
void run_listdef_tests(void);

#endif //LIBDS_TEST_RUNNER_H