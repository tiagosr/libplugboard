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

static t_plugclass
*inlet_class, *outlet_class, *loadbang_class, *patch_class, *bang_class,
*msg_class, *array_read_class, *array_write_class, *trigger_class;

typedef struct inlet_obj inlet_obj;
typedef struct outlet_obj outlet_obj;
typedef struct loadbang_obj loadbang_obj;
typedef struct patch_obj patch_obj;
typedef struct array_obj array_obj;
typedef struct msg_obj msg_obj;
typedef struct trigger_obj trigger_obj;

struct t_patch {
    t_sym name;
    inlet_obj *inlets_head, *inlets_tail;
    outlet_obj *outlet_head, *outlet_tail;
    loadbang_obj *loadbang_head, *loadbang_tail;
    patch_obj *instance_head, *instance_tail;
    t_patch *subpatch_head, *subpatch_tail;
    t_patch *prev, *next;
};

struct patch_obj {
    t_plugobj base;
    t_patch *patchref;
    patch_obj *prev, *next;
};

struct inlet_obj {
    t_plugobj base;
    t_patch *owner;
    inlet_obj *prev, *next;
};

static t_patch *current_patch;

struct outlet_obj {
    t_plugobj base;
    size_t pos;
    t_patch *owner;
    outlet_obj *prev, *next;
};

struct loadbang_obj {
    t_plugobj base;
    struct loadbang_obj *prev, *next;
    t_patch *owner;
};

t_patch* p_create_patch(void) {
    t_patch *patch = malloc(sizeof(t_patch));
    patch->inlets_head = patch->inlets_tail = NULL;
    patch->outlet_head = patch->outlet_tail = NULL;
    patch->instance_head = patch->instance_tail = NULL;
    patch->loadbang_head = patch->loadbang_tail = NULL;
    patch->name = NULL;
    patch->next = patch->prev = NULL;
    return patch;
}

static t_patch* find_subpatch(t_patch *patch, const char* name) {
    const char *ptr = name;
    while (ptr[0]) {
        if (ptr[0]=='/') {
            t_patch *temp;
            if (ptr==name) {
                temp = patch;
            } else {
                char *tempname = malloc(ptr+1-name);
                memcpy(tempname, name, ptr-name);
                tempname[ptr-name]=0;
                temp = find_subpatch(patch, tempname);
                free(tempname);
            }
            return find_subpatch(temp,ptr+1);
        }
        ptr++;
    }
    t_sym sym = find_sym(name);
    if (sym) {
        t_patch *subpatch = patch->subpatch_head;
        while (subpatch) {
            if (subpatch->name == sym) {
                return subpatch;
            }
            subpatch = subpatch->next;
        }
    }
    return NULL;
}

static void patchobj_inlet_perform(void *obj, t_any any, void *idata) {
    //patch_obj *pobj = (patch_obj *)obj;
    inlet_obj *iobj = (inlet_obj *)idata;
    p_outlet_send_any(iobj, 0, any);
    
}

static void* patchobj_create(int argc, char **argv) {
    patch_obj* obj = NULL;
    if (argc > 1) {
        t_patch *ref = find_subpatch(current_patch,argv[1]);
        if (ref) {
            obj = p_new(patch_class);
            obj->patchref = ref;
            if (ref->instance_tail) {
                ref->instance_tail->next = obj;
                obj->prev = ref->instance_tail;
            } else {
                ref->instance_head = obj;
                obj->prev = NULL;
            }
            obj->next = NULL;
            ref->instance_tail = obj;
        } else {
            t_patch *ref = p_create_patch();
            // find patch address if 
            if (current_patch->subpatch_tail) {
                current_patch->subpatch_tail->next = ref;
            } else {
                current_patch->subpatch_head = ref;
            }
            ref->prev = current_patch->subpatch_tail;
            ref->next = NULL;
            current_patch->subpatch_tail = ref;
            // create subpatch here
        }
    }
    return obj;
}

static void patchobj_destroy(t_plugobj *obj) {
    patch_obj* patch = (patch_obj *)obj;
    if (patch->prev) {
        patch->prev->next = patch->next;
    } else {
        patch->patchref->instance_head = patch->next;
    }
    if (patch->next) {
        patch->next->prev = patch->prev;
    } else {
        patch->patchref->instance_tail = patch->prev;
    }
    if (patch->patchref->instance_head == NULL && patch->patchref->instance_tail == NULL) {
        // destroy subpatch here
    }
}

static void* loadbang_create(int argc, char **argv) {
    loadbang_obj *obj = p_new(loadbang_class);
    p_add_outlet(obj, "bang");
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
static void loadbang_destroy(t_plugobj*obj) {
    loadbang_obj* loadbang = (loadbang_obj *)obj;
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
        p_outlet_bang(obj, 0);
        obj = obj->next;
    }
}

static void* inlet_create(int argc, char **argv) {
    inlet_obj *inlet = p_new(inlet_class);
    p_add_outlet(inlet, "incoming");
    if (current_patch->inlets_tail) {
        current_patch->inlets_tail->next = inlet;
    } else {
        current_patch->inlets_head = inlet;
    }
    inlet->prev = current_patch->inlets_tail;
    if (!current_patch->inlets_head) {
        current_patch->inlets_head = inlet;
    }
    return inlet;
}

static void inlet_destroy(t_plugobj* obj) {
    inlet_obj *inlet = (inlet_obj *)obj;
    if (inlet->prev) {
        inlet->prev->next = inlet->next;
    } else {
        inlet->owner->inlets_head = inlet->next;
    }
    if (inlet->next) {
        inlet->next->prev = inlet->prev;
    } else {
        inlet->owner->inlets_tail = inlet->prev;
    }
}

static void outlet_perform(void *obj, t_any any, void*idata) {
    outlet_obj* oobj = (outlet_obj *)obj;
    p_outlet_send_any(oobj->owner, oobj->pos, any);
}

static void* outlet_create(int argc, char **argv) {
    outlet_obj *obj = p_new(outlet_class);
    p_begin_inlet_fn_list(fns)
    p_inlet_fn_any(outlet_perform),
    p_end_inlet_fn_list
    p_add_inlet(obj, "outgoing", fns, NULL);
    return obj;
}

static void outlet_destroy(t_plugobj*obj) {
    outlet_obj *outlet = (outlet_obj *)obj;
    if (outlet->prev) {
        outlet->prev->next = outlet->next;
    } else {
        outlet->owner->outlet_head = outlet->next;
    }
    if (outlet->next) {
        outlet->next->prev = outlet->prev;
    } else {
        outlet->owner->outlet_tail = outlet->prev;
    }
}

typedef struct array_proper array_proper;
struct array_proper {
    int item_type;
    size_t array_size;
    size_t item_size;
    void *data;
    int retain_count;
};
static void array_send_item(array_obj *out, size_t outlet, array_proper *array, int index) {
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
            p_outlet_send_int(out, 0, ((int *)array->data)[p_index]);
            break;
        case t_type_float:
            p_outlet_send_float(out, 0, ((float *)array->data)[p_index]);
            break;
        /*
        case t_type_sym:
            p_outlet_send_sym(out, 0, ((t_sym *)array->data)[p_index]);
            break;
         */
        case t_type_string:
            p_outlet_send_string(out, 0, ((const char **)array->data)[p_index]);
            break;
        case t_type_any:
            p_outlet_send_any(out, 0, ((t_any *)array->data)[p_index]);
            break;
        default:
            break;
    }
}

struct array_obj {
    t_plugobj base;
    array_proper *array;
};

static t_sym range_sym;

static void array_read(void *obj, t_list command, void *idata) {
    array_obj *aobj = obj;
    switch (command.car_type) {
        case t_type_sym:
            if ((t_sym)command.car == range_sym) {
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
    array_send_item(aobj, 0, aobj->array, index);
}

static void *arrayread_create(int argc, char **argv) {
    array_obj *obj = p_new(array_read_class);
    p_begin_inlet_fn_list(fns)
    p_inlet_fn_list(array_read),
    p_inlet_fn_int(array_read_index),
    p_end_inlet_fn_list
    p_add_inlet(obj, "in", fns, NULL);
    p_add_outlet(obj, "out");
    return obj;
}



static void bang_perform(void *obj, void *idata)
{
    p_outlet_bang(obj, 0);
}

static void* bang_create(int argc, char **argv)
{
    t_plugobj *obj = p_new(bang_class);
    p_begin_inlet_fn_list(fns)
    p_inlet_fn_trigger(bang_perform),
    p_end_inlet_fn_list
    p_add_inlet(obj, "in", fns, NULL);
    p_add_outlet(obj, "out");
    return obj;
}

struct msg_obj {
    t_plugobj base;
    t_list msg_proto, msg;
};

static void msg_bang(void *obj, void *idata)
{
    msg_obj *mobj = obj;
    p_outlet_send_list(mobj, 0, mobj->msg);
}

static void *msg_create(int argc, char **argv)
{
    msg_obj *obj = NULL;
    if (argc>1) {
        obj = p_new(msg_class);
        
    }
    return obj;
}


void p_std_control_setup(void) {
    outlet_class = p_create_plugclass(gen_sym("outlet"), sizeof(outlet_obj),
                                      outlet_create, outlet_destroy, NULL, NULL);
    inlet_class = p_create_plugclass(gen_sym("inlet"), sizeof(inlet_obj),
                                     inlet_create, inlet_destroy, NULL, NULL);
    bang_class = p_create_plugclass(gen_sym("bang"), sizeof(t_plugobj),
                                    bang_create, NULL, NULL, NULL);
    loadbang_class = p_create_plugclass(gen_sym("loadbang"), sizeof(loadbang_obj),
                                        loadbang_create, loadbang_destroy, 0, 0);
    patch_class = p_create_plugclass(gen_sym("patch"), sizeof(patch_obj),
                                     patchobj_create, patchobj_destroy, 0, 0);
    array_read_class = p_create_plugclass(gen_sym("arrayread"), sizeof(array_obj),
                                          arrayread_create, NULL, NULL, NULL);
    range_sym = gen_sym("range");
}
