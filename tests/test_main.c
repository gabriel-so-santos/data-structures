/**
 * @file    test_nodechain.c
 * @brief   Test entry point
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "test_runner.h"

int main(int argc, char **argv)
{
    srand(time(NULL));

    run_nodechain_tests();
    run_listdef_tests();

    return EXIT_SUCCESS;
}