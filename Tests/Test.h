#ifndef _TEST_H
#define _TEST_H

typedef int(*test_fn_t)(int random);

typedef struct
{
    const char* name;
    test_fn_t fn;
} Test;

#endif // _TEST_H
