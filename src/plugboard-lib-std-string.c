//
//  plugboard-lib-std-string.c
//  libplugboard
//
//  Created by Tiago Rezende on 5/26/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-string.h"
#include <string.h>

static t_plugclass *strlen_class, *strcat_class;

typedef struct str_int_struct str_int_struct;
typedef struct str_op_struct str_op_struct;
typedef struct str_2op_struct str_2op_struct;

struct str_int_struct {
    t_plugobj base;
    int val;
};

struct str_op_struct {
    t_plugobj base;
    char *string;
};

static void str_op_destroy(void *obj)
{
    str_op_struct *op = obj;
    free(op->string);
}

struct str_2op_struct {
    t_plugobj base;
    char *stra, *strb;
};

static void str_2op_destroy(void *obj)
{
    str_2op_struct *op = obj;
    free(op->stra);
    free(op->strb);
}

static void str_2op_set_argument(void *obj, const char *str, void *idata)
{
    str_2op_struct *op = obj;
    if (op->strb) {
        free(op->strb);
    }
    op->strb = strdup(str);
}


// [strlen]
static void strlen_perform(void *obj, const char*str, void *idata)
{
    str_int_struct *op = obj;
    op->val = strlen(str);
    p_outlet_send_int(op, 0, op->val);
}

static void* strlen_create(int argc, char **argv)
{
    str_int_struct *op = p_new(strlen_class);
    op->val = 0;
    p_begin_inlet_fn_list(fstring)
    p_inlet_fn_string(strlen_perform),
    p_end_inlet_fn_list
    p_add_inlet(op, "string", fstring, NULL);
    p_add_outlet(op, "length");
    return op;
}
// end [strlen]


// [strcat]
static void strcat_perform(void *obj, const char *str, void *idata)
{
    str_2op_struct *op = obj;
    size_t len = strlen(str)+strlen(op->strb);
    if (op->stra) {
        free(op->stra);
    }
    op->stra = malloc(len+1);
    memcpy(op->stra, str, strlen(str)+1);
    op->stra = strcat(op->stra, op->strb);
    p_outlet_send_string(op, 0, op->stra);
}
static void strcat_perform_r(void *obj, const char *str, void *idata)
{
    str_2op_struct *op = obj;
    size_t len = strlen(str)+strlen(op->strb);
    if (op->stra) {
        free(op->stra);
    }
    op->stra = malloc(len+1);
    memcpy(op->stra, op->strb, strlen(op->strb)+1);
    op->stra = strcat(op->stra, str);
    p_outlet_send_string(op, 0, op->stra);
}

static void* strcat_create(int argc, char **argv)
{
    str_2op_struct *op = p_new(strcat_class);
    op->stra = strdup("");
    if (argc > 1) {
        op->strb = strdup((const char*)argv[1]);
    } else {
        op->strb = strdup("");
    }
    p_begin_inlet_fn_list(fcat)
        p_inlet_fn_string(strcat_perform),
    p_end_inlet_fn_list
    p_begin_inlet_fn_list(frcat)
        p_inlet_fn_string(strcat_perform_r),
    p_end_inlet_fn_list
    p_begin_inlet_fn_list(fset)
        p_inlet_fn_string(str_2op_set_argument),
    p_end_inlet_fn_list
    if (strcmp("strcat", (const char *)argv[0])==0)
    { p_add_inlet(op, "string", fcat, NULL); }
    else
    { p_add_inlet(op, "string", frcat, NULL); }
    p_add_inlet(op, "string", fset, NULL);
    p_add_outlet(op, "concatenated");
    return op;
}
// end [strcat]



// [char-at]

// end [char-at]



void p_std_string_setup(void)
{
    strlen_class = p_create_plugclass(gen_sym("strlen"), sizeof(str_int_struct),
                                      strlen_create, NULL, NULL, NULL);
    strcat_class = p_create_plugclass(gen_sym("strcat"), sizeof(str_2op_struct),
                                      strcat_create, NULL, NULL, NULL);
    p_create_class_alias(strcat_class, gen_sym("strcatr"));
}