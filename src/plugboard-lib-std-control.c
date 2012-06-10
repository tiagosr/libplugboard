//
//  plugboard-lib-std-control.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-control.h"
#include "plugboard-utils.h"
#include <stdio.h>
#include <stdlib.h>
#define POSIX_C_SOURCE 201205L
#include <string.h>

/*
 * control objects
 */
typedef struct array_obj array_obj;
typedef struct msg_obj msg_obj;
typedef struct trigger_obj trigger_obj;

extern t_patch *current_patch;

#pragma mark - [loadbang]
static t_plugclass *loadbang_class;
struct loadbang_obj {
    t_plugobj base;
    struct loadbang_obj *prev, *next;
    t_patch *owner;
    t_outlet *out;
};

static void* loadbang_create(int argc, char **argv) {
    loadbang_obj *obj = p_new(loadbang_class);
    obj->out = add_outlet(obj, "bang");
    obj->owner = current_patch;
    if (current_patch->loadbang_tail) {
        current_patch->loadbang_tail->next = obj;
        obj->prev = current_patch->loadbang_tail;
    } else {
        current_patch->loadbang_head = obj;
        obj->prev = NULL;
    }
    obj->next = NULL;
    current_patch->loadbang_tail = obj;
    return obj;
}
static void loadbang_destroy(void*obj) {
    loadbang_obj* loadbang = obj;
    if (loadbang->prev) {
        loadbang->prev->next = loadbang->next;        
    } else {
        loadbang->owner->loadbang_head = loadbang->next;
    }
    if (loadbang->next) {
        loadbang->next->prev = loadbang->prev;        
    } else {
        loadbang->owner->loadbang_tail = loadbang->prev;
    }
}

void p_perform_loadbang(t_patch *patch) {
    loadbang_obj *obj = patch->loadbang_head;
    while (obj) {
        o_bang(obj->out);
        obj = obj->next;
    }
}
// end [loadbang]

#pragma mark - [arrayread], [arraywrite]
static t_plugclass *array_read_class, *array_write_class;
typedef struct array_proper array_proper;
struct array_proper {
    int item_type;
    size_t array_size;
    size_t item_size;
    void *data;
    int retain_count;
};

#pragma mark [arrayread]
static void array_send_item(t_outlet *outlet, array_proper *array, int index) {
    size_t p_index;
    if (index >= 0 && index < array->array_size) {
        p_index = index;
    } else if (index < 0) {
        p_index = (array->array_size - index);
        if (p_index > array->array_size) {
            return;
        }
    }
    switch (array->item_type) {
        case t_type_int:
            o_int(outlet, ((int *)array->data)[p_index]);
            break;
        case t_type_float:
            o_float(outlet, ((float *)array->data)[p_index]);
            break;
        case t_type_string:
            o_string(outlet, ((const char **)array->data)[p_index]);
            break;
        case t_type_any:
            o_any(outlet, ((t_any *)array->data)[p_index]);
            break;
        default:
            break;
    }
}

struct array_obj {
    t_plugobj base;
    array_proper *array;
    t_outlet *out;
};

static t_sym range_sym;

static void array_read(void *obj, t_list command, void *idata) {
    array_obj *aobj = obj;
    switch (command.first.type) {
        case t_type_sym:
            if ((t_sym)command.first.sym == range_sym) {
                size_t start, end;
                end = aobj->array->array_size;
                start = 0;
            }
        default:
            break;
    }
}
static void array_read_index(void *obj, int index, void *idata) {
    array_obj *aobj = obj;
    array_send_item(aobj->out, aobj->array, index);
}

static void *arrayread_create(int argc, char **argv) {
    array_obj *obj = p_new(array_read_class);
    t_inlet *in = add_inlet(obj, "in", NULL);
    p_inlet_add_list_fn(in, array_read);
    p_inlet_add_int_fn(in, array_read_index);
    obj->out = add_outlet(obj, "out");
    return obj;
}

// end [arrayread]

#pragma mark [arraywrite]

#pragma mark - [bang]

static t_plugclass *bang_class;
typedef struct bang_obj {
    t_plugobj base;
    t_outlet *out;
} bang_obj;

static void bang_perform(void *obj, void *idata)
{
    o_bang(((bang_obj *)obj)->out);
}

static void* bang_create(int argc, char **argv)
{
    bang_obj *obj = p_new(bang_class);
    t_inlet *in = add_inlet(obj, "in", NULL);
    p_inlet_add_trigger_fn(in, bang_perform);
    obj->out = add_outlet(obj, "out");
    return obj;
}

// end [bang]

#pragma mark - [msg]
static t_plugclass *msg_class;
struct msg_obj {
    t_plugobj base;
    t_list msg_proto, msg;
    t_outlet *out;
};

static void msg_bang(void *obj, void *idata)
{
    msg_obj *mobj = obj;
    o_list(mobj->out, mobj->msg);
}

static void *msg_create(int argc, char **argv)
{
    msg_obj *obj = NULL;
    if (argc>1) {
        obj = p_new(msg_class);
        t_inlet *in = add_inlet(obj, "in", NULL);
        p_inlet_add_trigger_fn(in, msg_bang);
        obj->out = add_outlet(obj, "out");
        
    }
    return obj;
}
// end [msg]

#pragma mark - [trigger]

static t_plugclass *trigger_class;
typedef struct trigger_out trigger_out;
struct trigger_out {
    t_outlet *out;
    int type;
    trigger_out *next;
};
struct trigger_obj {
    t_plugobj base;
    trigger_out *head;
};

static void trigger_perform(void *obj, t_any any, void *idata)
{
    trigger_obj *trigger = obj;
    trigger_out *tout = trigger->head;
    char *str_out = NULL;
    switch (any.type) {
        case t_type_int:
            while (tout) {
                switch (tout->type) {
                    case t_type_int:
                        o_int(tout->out, any.i_data);
                        break;
                    case t_type_float:
                        o_float(tout->out, any.i_data);
                        break;
                    case t_type_string:
                        asprintf(&str_out, "%d", any.i_data);
                        o_string(tout->out, str_out);
                        free(str_out);
                        break;
                    case t_type_trigger:
                        o_bang(tout->out);
                    case t_type_any:
                        o_any(tout->out, any);
                    default:
                        break;
                }
                tout = tout->next;
            }
            break;
        case t_type_float:
            while (tout) {
                switch (tout->type) {
                    case t_type_int:
                        o_int(tout->out, any.f_data);
                        break;
                    case t_type_float:
                        o_float(tout->out, any.f_data);
                        break;
                    case t_type_string:
                        asprintf(&str_out, "%f", any.f_data);
                        o_string(tout->out, str_out);
                        free(str_out);
                        break;
                    case t_type_trigger:
                        o_bang(tout->out);
                    case t_type_any:
                        o_any(tout->out, any);
                    default:
                        break;
                }
                tout = tout->next;
            }
            break;
        case t_type_string:
            while (tout) {
                switch (tout->type) {
                    case t_type_int:
                        o_int(tout->out, atoi(any.string));
                        break;
                    case t_type_float:
                        o_float(tout->out, atof(any.string));
                        break;
                    case t_type_string:
                        o_string(tout->out, any.string);
                        break;
                    case t_type_trigger:
                        o_bang(tout->out);
                    case t_type_any:
                        o_any(tout->out, any);
                    default:
                        break;
                }
                tout = tout->next;
            }
            break;
        case t_type_list:
            while (tout) {
                switch (tout->type) {
                    case t_type_int:
                        switch (any.list->first.type) {
                            case t_type_int:
                                o_int(tout->out, any.list->first.i_data);
                                break;
                            case t_type_float:
                                o_int(tout->out, any.list->first.f_data);
                                break;
                            case t_type_string:
                                o_int(tout->out, atoi(any.list->first.string));
                                break;
                            case t_type_trigger:
                                o_int(tout->out, 1);
                                break;
                            default:
                                break;
                        }
                        break;
                    case t_type_float:
                        switch (any.list->first.type) {
                            case t_type_int:
                                o_float(tout->out, any.list->first.i_data);
                                break;
                            case t_type_float:
                                o_float(tout->out, any.list->first.f_data);
                                break;
                            case t_type_string:
                                o_float(tout->out, atof(any.list->first.string));
                                break;
                            case t_type_trigger:
                                o_float(tout->out, 1.0);
                                break;
                            default:
                                break;
                        }
                        break;
                    case t_type_string:
                        switch (any.list->first.type) {
                            case t_type_int:
                                asprintf(&str_out, "%d", any.i_data);
                                o_string(tout->out, str_out);
                                free(str_out);
                                break;
                            case t_type_float:
                                asprintf(&str_out, "%f", any.f_data);
                                o_string(tout->out, str_out);
                                free(str_out);
                                break;
                            case t_type_string:
#pragma message ("todo: [trigger]: list->string")
                                break;
                            case t_type_list:
#pragma message ("todo: [trigger]: implement list send")
                                //o_list(tout->out, *any.list);
                                break;
                            default:
                                break;
                        }
                        break;
                    case t_type_trigger:
                        o_bang(tout->out);
                    case t_type_any:
#pragma message ("todo: [trigger]: any->list")
                        o_any(tout->out, any);
                    default:
                        break;
                }
                tout = tout->next;
            }
            break;
        default:
            break;
    }
}

static void* trigger_create(int argc, char**argv) {
    trigger_obj *obj = p_new(trigger_class);
    trigger_out *prev = NULL, *tout = NULL;
    int count = 1;
    while (count<argc) {
        prev = tout;
        tout = malloc(sizeof(trigger_out));
        tout->next = prev;
        if ((strcmp("any", argv[count])==0)||(strcmp("a", argv[count])==0)) {
            tout->type = t_type_any;
            tout->out = add_outlet(obj, "any");
        } else if ((strcmp("int", argv[count])==0)||(strcmp("i", argv[count])==0)) {
            tout->type = t_type_int;
            tout->out = add_outlet(obj, "int");
        } else if ((strcmp("float", argv[count])==0)||(strcmp("float", argv[count])==0)) {
            tout->type = t_type_float;
            tout->out = add_outlet(obj, "float");
        } else if ((strcmp("string", argv[count])==0)||(strcmp("s", argv[count])==0)) {
            tout->type = t_type_string;
            tout->out = add_outlet(obj, "string");
        } else if ((strcmp("list", argv[count])==0)||(strcmp("l", argv[count])==0)) {
            tout->type = t_type_list;
            tout->out = add_outlet(obj, "list");
        } else if ((strcmp("bang", argv[count])==0)||(strcmp("b", argv[count])==0)) {
            tout->type = t_type_trigger;
            tout->out = add_outlet(obj, "bang");
        }
        count++;
    }
    obj->head = tout;
    t_inlet *inlet = add_inlet(obj, "in", NULL);
    p_inlet_add_any_fn(inlet, trigger_perform);
    return obj;
}

static void trigger_destroy(void *obj) {
    trigger_obj *trigger = obj;
    trigger_out *out = trigger->head, *current;
    while (out) {
        current = out;
        out = out->next;
        free(current);
    }
}

// end [trigger]

#pragma mark - [select]

// end [select]

#pragma mark - [pack]

// end [pack]

#pragma mark - [unpack]

// end [unpack]

#pragma mark - [route]

// end [route]

#pragma mark - [send]

// end [send]

#pragma mark - [receive]

// end [receive]


#pragma mark - control library setup
void p_std_control_setup(void) {
    bang_class = p_create_plugclass(gen_sym("bang"), sizeof(bang_obj),
                                    bang_create, NULL, NULL, NULL);
    loadbang_class = p_create_plugclass(gen_sym("loadbang"), sizeof(loadbang_obj),
                                        loadbang_create, loadbang_destroy, 0, 0);
    array_read_class = p_create_plugclass(gen_sym("arrayread"), sizeof(array_obj),
                                          arrayread_create, NULL, NULL, NULL);
    range_sym = gen_sym("range");
    trigger_class = p_create_plugclass(gen_sym("trigger"), sizeof(trigger_obj),
                                       trigger_create, trigger_destroy, NULL, NULL);
    p_create_class_alias(trigger_class, gen_sym("t"));
}
