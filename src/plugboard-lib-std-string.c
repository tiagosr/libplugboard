//
//  plugboard-lib-std-string.c
//  libplugboard
//
//  Created by Tiago Rezende on 5/26/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-string.h"
#include <string.h>
#include <stdio.h>


// [strlen]
static t_plugclass *strlen_class;
typedef struct strlen_struct {
    t_plugobj base;
    int len;
    t_outlet *out;
} strlen_struct;

static void strlen_perform(void *obj, const char*str, void *idata)
{
    strlen_struct *op = obj;
    op->len = strlen(str);
    o_int(op->out, op->len);
}

static void* strlen_create(int argc, char **argv)
{
    strlen_struct *op = p_new(strlen_class);
    op->len= 0;
    inlet_fn_list(fstring)
    i_fn_string(strlen_perform),
    inlet_fn_list_end
    add_inlet(op, "string", fstring, NULL);
    op->out = add_outlet(op, "length");
    return op;
}
// end [strlen]


// [strcat]
static t_plugclass *strcat_class;
typedef struct strcat_struct {
    t_plugobj base;
    char *result, *to_concat;
    t_outlet *out;
} strcat_struct;

static void strcat_destroy(void *obj)
{
    strcat_struct *op = obj;
    free(op->result);
    free(op->to_concat);
}

static i_string_setter(strcat_set_to_concat, strcat_struct, to_concat);

static void strcat_perform(void *obj, const char *str, void *idata)
{
    strcat_struct *op = obj;
    size_t len = strlen(str)+strlen(op->to_concat);
    if (op->result) {
        free(op->result);
    }
    op->result = malloc(len+1);
    memcpy(op->result, str, strlen(str)+1);
    op->result = strcat(op->result, op->to_concat);
    o_string(op->out, op->result);
}
static void strcat_perform_r(void *obj, const char *str, void *idata)
{
    strcat_struct *op = obj;
    size_t len = strlen(str)+strlen(op->to_concat);
    if (op->result) {
        free(op->result);
    }
    op->result = malloc(len+1);
    memcpy(op->result, op->to_concat, strlen(op->to_concat)+1);
    op->result = strcat(op->result, str);
    o_string(op->out, op->result);
}

static void* strcat_create(int argc, char **argv)
{
    strcat_struct *op = p_new(strcat_class);
    op->result = strdup("");
    if (argc > 1) {
        op->to_concat = strdup((const char*)argv[1]);
    } else {
        op->to_concat = strdup("");
    }
    inlet_fn_list(fcat)
        i_fn_string(strcat_perform),
    inlet_fn_list_end
    inlet_fn_list(frcat)
        i_fn_string(strcat_perform_r),
    inlet_fn_list_end
    inlet_string_fn(fset, strcat_set_to_concat);
    if (strcmp("strcat", (const char *)argv[0])==0)
    { add_inlet(op, "string", fcat, NULL); }
    else
    { add_inlet(op, "string", frcat, NULL); }
    add_inlet(op, "string", fset, NULL);
    op->out = add_outlet(op, "concatenated");
    return op;
}

// end [strcat]



// [char-at]
static t_plugclass *char_at_class;
typedef struct char_at_struct {
    t_plugobj base;
    int pos;
    int character;
    t_outlet *out;
} char_at_struct;

static void char_at_perform(void *obj, const char *string, void *idata)
{
    char_at_struct *c = obj;
    size_t str_len = strlen(string);
    if (c->pos < str_len && c->pos >= 0) {
        o_int(c->out, c->character = string[c->pos]);
    } else if (c->pos < 0 && c->pos >= -str_len) {
        o_int(c->out, c->character = string[str_len - c->pos]);
    }
}

static i_setter(char_at_set_pos, int, char_at_struct, pos);

static void* char_at_create(int argc, char**argv)
{
    char_at_struct *obj = p_new(char_at_class);
    if (argc>1) {
        obj->pos = atoi(argv[1]);
    } else {
        obj->pos = 0;
    }
    inlet_fn_list(fn)
        i_fn_string(char_at_perform),
    inlet_fn_list_end
    inlet_fn_list(fn2)
        i_fn_int(char_at_set_pos),
    inlet_fn_list_end
    add_inlet(obj, "string", fn, NULL);
    add_inlet(obj, "position", fn2, NULL);
    obj->out = add_outlet(obj, "character");
    return obj;
}

// end [char-at]

// [substr]
static t_plugclass *substr_class;

typedef struct substr_struct {
    t_plugobj base;
    char *part;
    int from, to;
    t_outlet *out;
} substr_struct;

static void substr_perform(void *vobj, const char* string, void *idata)
{
    substr_struct *obj = vobj;
    int len = strlen(string);
    int start = obj->from;
    if (start < 0) {
        if (start < -len) {
            start = 0;
        } else {
            start = len - start;
            len -= start;
        }
    } else if (start > len) {
        start = len;
        len = 0;
    }
    if (obj->to >= 0) {
        if (len > obj->to) {
            len = obj->to;
        }
    } else {
        if (obj->to < -len) {
            len = 0;
        } else {
            len += (obj->to + 1);
        }
    }
    free(obj->part);
    if (len == 0) {
        obj->part = strdup("");
    } else {
        obj->part = malloc(len+1);
        memcpy(obj->part, string+start, len);
    }
    o_string(obj->out, obj->part);
}

static i_setter(substr_set_from, int, substr_struct, from);
static i_setter(substr_set_to, int, substr_struct, to);

static void* substr_create(int argc, char** argv) {
    substr_struct *obj = p_new(substr_class);
    if (argc > 1) {
        if (argc > 2) {
            obj->to = atoi(argv[2]);
        } else {
            obj->to = -1;
        }
        obj->from = atoi(argv[1]);
    } else {
        obj->from = 0;
        obj->to = -1;
    }
    obj->part = strdup("");
    inlet_fn_list(fnstring)
    i_fn_string(substr_perform),
    inlet_fn_list_end
    inlet_int_fn(fnsetfrom, substr_set_from);
    inlet_int_fn(fnsetto, substr_set_to);
    
    add_inlet(obj, "string", fnstring, NULL);
    add_inlet(obj, "from", fnsetfrom, NULL);
    add_inlet(obj, "to", fnsetto, NULL);
    
    obj->out = add_outlet(obj, "substring");
    return obj;
}

static void substr_destroy(void *obj) {
    substr_struct *sobj = obj;
    free(sobj->part);
}

// end [substr]

// [itoa]
static t_plugclass *itoa_class;
typedef struct itoa_obj {
    t_plugobj base;
    char *result;
    t_outlet *out;
} itoa_obj;

static void itoa_perform(void *obj, int num, void *idata)
{
    itoa_obj *iobj = obj;
    free(iobj->result);
    iobj->result = NULL;
    asprintf(&iobj->result, "%d",num);
    o_string(iobj->out, iobj->result);
}

static void* itoa_create(int argc, char **argv)
{
    itoa_obj *obj = p_new(itoa_class);
    obj->result = strdup("");
    inlet_int_fn(fn, itoa_perform);
    add_inlet(obj, "int", fn, NULL);
    obj->out = add_outlet(obj, "string");
    return obj;
}

static void itoa_destroy(void *obj)
{
    free(((itoa_obj *)obj)->result);
}

// end [itoa]




void p_std_string_setup(void)
{
    strlen_class = p_create_plugclass(gen_sym("strlen"), sizeof(strlen_struct),
                                      strlen_create, NULL, NULL, NULL);
    
    strcat_class = p_create_plugclass(gen_sym("strcat"), sizeof(strcat_struct),
                                      strcat_create, strcat_destroy, NULL, NULL);
    p_create_class_alias(strcat_class, gen_sym("strcatr"));

    char_at_class = p_create_plugclass(gen_sym("char-at"), sizeof(char_at_struct), 
                                       char_at_create, NULL, NULL, NULL);
    
    substr_class = p_create_plugclass(gen_sym("substr"), sizeof(substr_struct),
                                      substr_create, substr_destroy, NULL, NULL);
    
    itoa_class = p_create_plugclass(gen_sym("itoa"), sizeof(itoa_obj), 
                                    itoa_create, itoa_destroy, NULL, NULL);
    p_create_class_alias(itoa_class, gen_sym("i->s"));
    
    
}