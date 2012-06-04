//
//  plugboard-lib-std-basictypes.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-basictypes.h"
#include "plugboard-utils.h"
#include <stdio.h>
#include <string.h>


/*
 * basic types and containers
 */

// [int], [i]
static t_plugclass *int_class;
typedef struct int_obj {
    t_plugobj base;
    int data;
    t_outlet *out;
} int_obj;

static void int_set(void *obj, t_any data, void*idata) {
    int_obj *iobj = (int_obj *)obj;
    if (data.type == t_type_int) {
        iobj->data = data.i_data;
    } else if (data.type == t_type_float) {
        iobj->data = data.f_data;
    } else if (data.type == t_type_string || data.type == t_type_sym) {
        int ival = 0;
        if (sscanf(data.string, "%d", &ival)>0) {
            iobj->data = ival;
        }
    }
    o_int(iobj->out, iobj->data);
}
static void *i_class_new(int argc, char **argv) {
    int_obj *obj = p_new(int_class);
    obj->data = 0;
    inlet_fn_list(fns)
    i_fn_any(int_set),
    inlet_fn_list_end
    add_inlet(obj, "in", fns, NULL);
    obj->out = add_outlet(obj, "out");
    return obj;
}

// [float], [f]
static t_plugclass *float_class;
typedef struct {
    t_plugobj base;
    float data;
    t_outlet *out;
} float_obj;

static void f_set(void *obj, t_any data, void *idata) {
    float_obj *fobj = (float_obj *)obj;
    if (data.type == t_type_int) {
        fobj->data = data.i_data;
    } else if (data.type == t_type_float) {
        fobj->data = data.f_data;
    } else if (data.type == t_type_string || data.type == t_type_sym) {
        float ival = 0;
        if (sscanf(data.string, "%f", &ival)>0) {
            fobj->data = ival;
        }
    }
    o_float(fobj->out, fobj->data);
}
static void *f_class_new(int argc, char **argv) {
    float_obj *obj = p_new(int_class);
    obj->data = 0;
    inlet_fn_list(fns)
    i_fn_any(f_set),
    inlet_fn_list_end
    add_inlet(obj, "in", fns, NULL);
    obj->out = add_outlet(obj, "out");
    return obj;
}

// [string], [s]
static t_plugclass *string_class;
typedef struct string_obj {
    t_plugobj base;
    char *string;
    t_outlet *out;
} string_obj;
static void s_set(void *obj, t_any data, void*idata) {
    char *tmp = NULL;
	switch (data.type) {
		case t_type_int:
		case t_type_float:
            break;
		case t_type_string:
            tmp = strdup(data.string);
            free(((string_obj *)obj)->string);
            ((string_obj *)obj)->string = tmp;
            break;
        default:
            break;
	}
    o_string(((string_obj *)obj)->out, ((string_obj *)obj)->string);
}
static void* s_create(int argc, char** argv) {
	string_obj* obj = p_new(string_class);
	if(argc > 1) {
		obj->string = strdup(argv[1]);
	} else {
		obj->string = strdup("");
	}
	inlet_fn_list(fns)
    i_fn_any(s_set),
    inlet_fn_list_end
    add_inlet(obj, "in", fns, NULL);
	obj->out = add_outlet(obj, "out");
	return obj;
}

// [list], [l]
static t_plugclass *list_class;
typedef struct list_obj {
    t_plugobj base;
    t_list list;
    t_outlet *out;
} list_obj;

static void list_set(void *obj, t_any any, void*idata) {
    list_obj *lobj = (list_obj *)obj;
    if (any.type == t_type_trigger) {
        
    } else if (any.type == t_type_list) {
        memcpy(&(lobj->list), any.list, sizeof(t_list));
    } else {
        lobj->list.car_type = any.type;
        lobj->list.car = any.something;
    }
    o_list(lobj->out, lobj->list);
}

static void* list_create(int argc, const char **argv) {
    list_obj *obj = p_new(list_class);
    
    inlet_fn_list(fns)
    i_fn_any(list_set),
    inlet_fn_list_end
    add_inlet(obj, "in", fns, NULL);
    obj->out = add_outlet(obj, "out");
    return obj;
}

// [any], [a]
static t_plugclass *any_class;
typedef struct any_obj {
	t_plugobj base;
	t_any any;
    t_outlet *out;
} any_obj;
static void any_set(void *obj, t_any any, void*idata) {
	any_obj *a = (any_obj*)obj;
	if (any.type != t_type_trigger) {
		a->any = any;
	}
	o_any(a->out, a->any);
}
static void* any_create(int argc, const char **argv) {
	any_obj* obj = p_new(any_class);
	obj->any = (t_any){t_type_trigger, 0};
	inlet_fn_list(fns)
    i_fn_any(any_set),
    inlet_fn_list_end
    add_inlet(obj, "in", fns, NULL);
	obj->out = add_outlet(obj, "out");
	return obj;
}


void p_std_basictypes_setup(void) {
    int_class = p_create_plugclass(gen_sym("int"), sizeof(int_obj),
                                   i_class_new,p_default_destroy,0,0);
    p_create_class_alias(int_class, gen_sym("i"));
    float_class = p_create_plugclass(gen_sym("float"), sizeof(float_obj),
                                     f_class_new,p_default_destroy,0,0);
    p_create_class_alias(int_class, gen_sym("f"));
    string_class = p_create_plugclass(gen_sym("string"), sizeof(string_obj),
                                      s_create,p_default_destroy,0,0);
    p_create_class_alias(string_class, gen_sym("s"));
}