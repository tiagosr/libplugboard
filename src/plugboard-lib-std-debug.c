//
//  plugboard-lib-std-debug.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-debug.h"
#include "plugboard-utils.h"
#include <stdio.h>
#include <string.h>

/*
 * basic graphic/debug/diagnosis types
 */
// [print]
static t_plugclass *print_class;
//*msg_class, *numbox_class, *strbox_class,
//*bng_class, *tgl_class
static void(*internal_print_hook)(const char*string);
void p_print_set_hook(void(*print_hook)(const char*string))
{
    internal_print_hook = print_hook;
}

typedef struct print_data_obj {
    t_plugobj base;
    char * description;
} print_data_obj;

static void print_perform(void *obj, t_any data, void*idata)
{
    
    if (internal_print_hook) {
        char *part_message;
        size_t partsize = 0;
        switch (data.type) {
            case t_type_int:
                asprintf(&part_message, "%d",data.i_data);
                break;
            case t_type_float:
                asprintf(&part_message, "%f",data.f_data);
                break;
            case t_type_sym:
            case t_type_string:
                part_message = strdup(data.string);
                break;
            case t_type_trigger:
                part_message = strdup("bang!");
                break;
            default:
                asprintf(&part_message, "<type %d object at 0x%X>", data.type, (unsigned)data.something);
                break;
        }
        print_data_obj *pobj = obj;
        char *message;
        if (pobj->description) {
            asprintf(&message, "%s: %s",pobj->description, part_message);
        } else {
            message = strdup(part_message);
        }
        internal_print_hook(message);
        if (part_message) free(part_message);
        free(message);
    }
}

static void*print_create(int argc, char **argv) {
    print_data_obj*obj = p_new(print_class);
    if (argc>1) {
        obj->description = strdup(argv[1]);
    } else {
        obj->description = NULL;
    }
    p_begin_inlet_fn_list(fns)
    p_inlet_fn_any(print_perform),
    p_end_inlet_fn_list
    p_add_inlet(obj, "in", fns, NULL);
    return obj;
}


void p_std_debug_setup(void) {
    print_class = p_create_plugclass(gen_sym("print"), sizeof(print_data_obj),
                                     print_create, p_default_destruct, 0, 0);
    p_create_class_alias(print_class, gen_sym("p"));
}
