//
//  plugboard.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 4/30/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "plugboard-utils.h"
void p_std_setup(void);
typedef struct t_plugclass_name {
    t_sym name; t_plugclass *ref;
} t_plugclass_name;


/**
 *
 * statics and globals
 *
 */

//static unsigned stack_depth = 0;
static int symbol_pool_size = -1, symbol_pool_occupied = 0;
static t_sym * symbol_pool = NULL;

static int class_pool_size = -1, class_pool_occupied = 0;
static t_plugclass_name *class_pool = NULL;
static t_sym bang_sym = NULL;
static t_plugclass
*inlet_class, *outlet_class, *patch_class;

t_patch *current_patch;

#pragma mark - Symbol stuff

/*
 * generates a symbol by putting a copy of the string in the symbol pool,
 * if not already there. otherwise sends previously stored string's pointer.
 */
t_sym gen_sym(const char *s)
{
    if (!symbol_pool) {
        symbol_pool = malloc(sizeof(t_sym)*128);
        if (!symbol_pool) {
            fprintf(stderr, "ERROR: Symbol pool could not be created!\n");
            return NULL;
        }
        memset(symbol_pool, sizeof(t_sym)*128, 0);
        symbol_pool_size = 128;
        symbol_pool_occupied = 0;
    }
    for (int i = 0; i < symbol_pool_occupied; i++) {
        if (strcmp(symbol_pool[i], s)==0) {
            return symbol_pool[i];
        } 
    }
    if (symbol_pool_occupied >= symbol_pool_size) {
        symbol_pool = realloc(symbol_pool,sizeof(char*)*(symbol_pool_size+128));
        if (!symbol_pool) {
            fprintf(stderr, "ERROR: Symbol pool could not be grown! Last pool size: %d bytes\n", symbol_pool_size);
            return NULL;
        }
        memset(symbol_pool+symbol_pool_size, 128*sizeof(t_sym**), 0);
        symbol_pool_size += 128;
    }
    symbol_pool[symbol_pool_occupied] = strdup(s);
    return symbol_pool[symbol_pool_occupied++];
}

/*
 * tries to find symbol in the pool, returning it's pointer if found, null otherwise.
 */
t_sym find_sym(const char *s)
{
    if (!symbol_pool) {
        return NULL;
    }
    for (int i = 0; i < symbol_pool_occupied; i++) {
        if (strcmp(symbol_pool[i], s)==0) {
            return symbol_pool[i];
        }
    }
    return NULL;
}

#pragma mark - Outlet Sentry Utility Functions

/*
 * utility functions for sending data to outlets
 */
static void send_int_as_string(void*target, t_inlet_fn *in, int data, void * tg_data)
{
    size_t str_len = snprintf(NULL,0, "%d",data);
    char *str_data = malloc(str_len);
    snprintf(str_data, str_len, "%d",data);
    in->inlet_string(target,str_data,tg_data);
    free(str_data);
}

static void send_float_as_string(void*target, t_inlet_fn *in, float data, void * tg_data)
{
    size_t str_len = snprintf(NULL, 0, "%f", data);
    char *str_data = malloc(str_len);
    snprintf(str_data, str_len, "%f", data);
    in->inlet_string(target,str_data,tg_data);
    free(str_data);
}

static void send_int_as_any(void*target, t_inlet_fn *in, int data, void * tg_data)
{
    t_any any;
    any.type = t_type_int;
    any.i_data = data;
    in->inlet_any(target,any,tg_data);
}

static void send_float_as_any(void*target, t_inlet_fn *in, float data, void * tg_data)
{
    t_any any;
    any.type = t_type_float;
    any.f_data = data;
    in->inlet_any(target,any,tg_data);
}

#pragma mark - Outlet sentry
/*
 * outlet sentry functions
 */


void p_outlet_send_int(t_outlet *outlet, int data)
{
    t_outlet_connection *current = outlet->head;
    while (current) {
        /* try sending to the given inlet as one of the basic types,
         * in the following order:
         * int, float, string, any, trigger
         * once a suitable inlet function is found, send and proceed
         * to the next inlet
         */
        t_inlet_fn *inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_int) {
                inlet->inlet_int(current->target,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_float) {
                inlet->inlet_float(current->target,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_any) {
                send_int_as_any(current->target,inlet,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_string) {
                send_int_as_string(current->target,inlet,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_trigger) {
                inlet->inlet_trigger(current->target,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
    sent:
        current=current->next;
    }
}

void p_outlet_send_float(t_outlet *outlet, float data)
{
    t_outlet_connection *current = outlet->head;
    while (current) {
        t_inlet_fn *inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_float) {
                inlet->inlet_float(current->target,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_any) {
                send_float_as_any(current->target,inlet,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_string) {
                send_float_as_string(current->target,inlet,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_int) {
                inlet->inlet_int(current->target,data,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_trigger) {
                inlet->inlet_trigger(current->target,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
    sent:
        current=current->next;
    }
}


void p_outlet_send_string(t_outlet *outlet, const char* string)
{
    t_outlet_connection *current = outlet->head;
    while (current) {
        t_inlet_fn *inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_string) {
                inlet->inlet_string(current->target,string,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        t_any any_sym = {t_type_sym, .string=(char *)string};
        while (inlet) {
            if (inlet->inlet_type == t_type_any) {
                inlet->inlet_any(current->target,any_sym,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_trigger) {
                inlet->inlet_trigger(current->target,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
    sent:
        current=current->next;
    }
}

void p_outlet_send_any(t_outlet *outlet, t_any any)
{
    t_outlet_connection *current = outlet->head;
    while (current) {
        t_inlet_fn *inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_string) {
                inlet->inlet_any(current->target,any,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (any.type == inlet->inlet_type) {
                switch (any.type) {
                    case t_type_int:
                        inlet->inlet_int(current->target,any.i_data,current->inlet->data);
                        goto sent;
                    case t_type_float:
                        inlet->inlet_float(current->target,any.f_data,current->inlet->data);
                        goto sent;
                    case t_type_string:
                        inlet->inlet_string(current->target,any.string,current->inlet->data);
                        goto sent;
                    case t_type_list:
                        inlet->inlet_list(current->target,*any.list,current->inlet->data);
                        goto sent;
                }
            }
            inlet=inlet->next;
        }
        inlet = current->inlet->head;
        while (inlet) {
            if (inlet->inlet_type == t_type_trigger) {
                inlet->inlet_trigger(current->target,current->inlet->data);
                goto sent;
            }
            inlet=inlet->next;
        }
    sent:
        current=current->next;
    }
}

void p_outlet_bang(t_outlet *outlet)
{
    t_outlet_connection *current = outlet->head;
    while (current) {
        t_inlet_fn *inlet = current->inlet->head;
        while (inlet) {
            switch (inlet->inlet_type) {
                case t_type_trigger:
                    inlet->inlet_trigger(current->target,current->inlet->data);
                    break;
                /*
                case t_type_sym:
                    inlet[0]->inlet_sym(current[0]->target,bang_sym,
                                                current[0]->inlet.data);
                    break;
                */
                case t_type_string:
                    inlet->inlet_string(current->target,bang_sym,
                                                   current->inlet->data);
                    break;
                default:
                    inlet->inlet_any(current->target,(t_any){t_type_trigger,0},
                                                current->inlet->data);
            }
            inlet=inlet->next;
        }
        current=current->next;
    }
}

void p_send_redraw(t_plugobj *obj, t_any data)
{
    if (obj && obj->prototype->redraw) {
        obj->prototype->redraw(obj,data,obj->prototype->classdata);
    }
}

#pragma mark - Inlet creation
/*
 * inlet creation
 */
t_inlet* p_add_inlet(t_plugobj *obj, t_sym name, void *idata)
{
    t_inlet *inlet = malloc(sizeof(t_inlet));
    inlet->name = name;
    inlet->data = idata;
    inlet->head = NULL;
    inlet->tail = NULL;
    inlet->prev = obj->in_tail;
    inlet->next = NULL;
    if(obj->in_tail) {
        obj->in_tail->next = inlet;
        obj->in_tail = inlet;
    } else {
        obj->in_head = obj->in_tail = inlet;
    }
    obj->inlet_count++;
    return inlet;
}

static void p_inlet_add_fn(t_inlet *inlet, int inlet_type, void *fn) {
    t_inlet_fn *i_fn = malloc(sizeof(t_inlet_fn));
    i_fn->next = NULL;
    i_fn->inlet_type = inlet_type;
    i_fn->inlet_any = (t_inlet_any_fn)fn;
    if (inlet->tail) {
        inlet->tail->next = i_fn;
    } else {
        inlet->head = inlet->tail = i_fn;
    }
    inlet->tail = i_fn;    
}

void p_inlet_add_trigger_fn(t_inlet *inlet, t_inlet_trigger_fn fn)
{
    p_inlet_add_fn(inlet, t_type_trigger, fn);
}

void p_inlet_add_int_fn(t_inlet *inlet, t_inlet_int_fn fn)
{
    p_inlet_add_fn(inlet, t_type_int, fn);
}

void p_inlet_add_float_fn(t_inlet *inlet, t_inlet_float_fn fn)
{
    p_inlet_add_fn(inlet, t_type_float, fn);
}

void p_inlet_add_string_fn(t_inlet *inlet, t_inlet_string_fn fn)
{
    p_inlet_add_fn(inlet, t_type_string, fn);
}

void p_inlet_add_list_fn(t_inlet *inlet, t_inlet_list_fn fn)
{
    p_inlet_add_fn(inlet, t_type_list, fn);
}

void p_inlet_add_any_fn(t_inlet *inlet, t_inlet_any_fn fn)
{
    p_inlet_add_fn(inlet, t_type_any, fn);
}


void p_inlet_set_tag(t_inlet *inlet, int tag) {
    inlet->tag = tag;
}

int p_inlet_get_tag(t_inlet *inlet) {
    return inlet->tag;
}


#pragma mark - Outlet creation
/*
 * outlet creation
 */

t_outlet * p_add_outlet(t_plugobj *obj, t_sym name)
{
    t_outlet *outlet = malloc(sizeof(t_outlet));
    outlet->name = name;
    outlet->head = outlet->tail = NULL;
    outlet->prev = obj->out_tail;
    outlet->next = NULL;
    if (obj->out_tail) {
        obj->out_tail->next = outlet;
        obj->out_tail = outlet;
    } else {
        obj->out_head = obj->out_tail = outlet;
    }
    obj->outlet_count++;
    return outlet;
}


/*
 * embed-side utilities
 */
t_list p_list_cons(int16_t a_type, void* a_data, int16_t b_type, void* b_data)
{
    t_list temp = {{a_type, .something=a_data}, malloc(sizeof(t_list))};
    temp.rest->first.type = b_type;
    temp.rest->first.something = b_data;
    temp.rest->rest = NULL;
    return temp;
}

#pragma mark - Class creation

static void p_default_destroy(void *_) {}
static void p_default_redraw(void *_, t_any ___, void *__) {}


/*
 * class creation and setup
 */

t_plugclass* p_create_plugclass(t_sym symbol, size_t object_size,
                                t_plugobj_creator_fn creator,
                                t_plugobj_destructor_fn destructor,
                                t_plugobj_redraw_fn redraw,
                                void *classdata)
{
    if (!class_pool) {
        class_pool = malloc(sizeof(t_plugclass_name)*128);
        if (!class_pool) {
            fprintf(stderr, "ERROR: Could not create class pool");
            return NULL;
        }
        memset(class_pool, 128*sizeof(t_plugclass_name*), 0);
        class_pool_occupied = 0;
        class_pool_size = 128;
    }
    t_plugclass *pc = malloc(sizeof(t_plugclass));
    if (!pc) {
        fprintf(stderr, "ERROR: Could not allocate new class object");
        return NULL;
    }
    pc->classname = symbol;
    pc->creator = creator;
    pc->destructor = p_default_destroy;
    if (destructor) {
        pc->destructor = destructor;
    }
    pc->redraw = p_default_redraw;
    if (redraw) {
        pc->redraw = redraw;
    }
    pc->classdata = classdata;
    pc->obj_size = object_size;
    if (class_pool_occupied >= class_pool_size) {
        class_pool = realloc(class_pool, sizeof(t_plugclass_name)*(class_pool_size+128));
        if (!class_pool) {
            fprintf(stderr, "ERROR: Could not grow class pool");
            return NULL;            
        }
        memset(class_pool+class_pool_occupied, 128*sizeof(t_plugclass_name*), 0);
        class_pool_size += 128;
    }
    class_pool[class_pool_occupied++] = (t_plugclass_name){symbol, pc};
    return pc;
}



void p_create_class_alias(t_plugclass *ref, t_sym alias) {
    if (class_pool_occupied >= class_pool_size) {
        class_pool = realloc(class_pool, sizeof(t_plugclass_name)*(class_pool_size+128));
        if (!class_pool) {
            fprintf(stderr, "ERROR: Could not grow class pool");
        }
        memset(class_pool+class_pool_occupied, 128*sizeof(t_plugclass_name*), 0);
        class_pool_size += 128;
    }
    class_pool[class_pool_occupied++] = (t_plugclass_name){alias, ref};
}

#pragma mark - Object creation

void* p_new(t_plugclass *class_spec)
{
    t_plugobj* temp = malloc(class_spec->obj_size);
    temp->prototype = class_spec;
    temp->in_head = temp->in_tail = NULL;
    temp->out_head = temp->out_tail = NULL;
    temp->inlet_count = temp->outlet_count = 0;
    return temp;
}



t_args p_separate_args(char *string)
{
    int argc = 1;
    char **argv = malloc(sizeof(char*)*argc+1);
    char *args = string;
    argv[0] = args;
    argv[1] = NULL;
    size_t argp = 0;
    int in_string = 0;
    int parens = 0;
    int skip = 0;
    char *prev_ptr = NULL;
    while (args[argp]!=0) {
        if (skip>0) {
            skip--;
        } else {
            switch (args[argp]) {
                case '\\':
                    skip++;
                    break;
                case '"':
                    in_string = in_string?0:1;
                    break;
                case ' ':
                case '\t':
                case '\n':
                    if (!in_string||!parens) {
                        args[argp]=0;
                        if (prev_ptr==args+argp) {
                            argv[argc-1] = &args[argp+1];
                        } else {
                            argc++;
                            argv = realloc(argv, sizeof(char*)*argc+1);
                            argv[argc-1] = &args[argp+1];
                            argv[argc] = 0;
                        }
                        prev_ptr = &args[argp+1];
                    }
                    break;
                case '(':
                case '[':
                case '{':
                    parens++;
                    break;
                case '}':
                case ']':
                case ')':
                    parens--;
                    break;
                default:
                    break;
            }
        }
        argp++;
    }
    return (t_args){argc, argv};
}

t_plugobj* p_create_plugobj(const char* spec, void* drawdata)
{
    char *tosplit = strdup(spec);
    t_args args = p_separate_args(tosplit);
    t_sym sym = find_sym(args.argv[0]);
    t_plugobj *res = NULL;
    if (sym) {
        for (int i = 0; i < class_pool_occupied; i++) {
            if (class_pool[i].name == sym) {
                res = class_pool[i].ref->creator(args.argc, args.argv);
            }
        }
    }
    free(args.argv);
    free(tosplit);
    if (res) {
        res->drawdata = drawdata;
    }
    return res;
}

void p_destroy(t_plugobj *obj) {
    obj->prototype->destructor(obj);
    t_inlet *in = obj->in_head;
    while (in) {
        t_inlet *tmp = in;
        in = in->next;
        t_inlet_fn *fn = tmp->head;
        while (fn) {
            t_inlet_fn *fn_tmp = fn;
            fn = fn->next;
            free(fn_tmp);
        }
        free(tmp);
    }
    t_outlet *out = obj->out_head;
    while (out) {
        t_outlet *tmp = out;
        out = out->next;
        t_outlet_connection *conn = tmp->head;
        while (conn) {
            t_outlet_connection *conn_tmp = conn;
            conn = conn->next;
            free(conn_tmp);
        }
        free(tmp);
    }
    free(obj);
}

#pragma mark - Object connection

/*
 * connecting stuff up
 */

int p_connect(t_plugobj *from, int outlet, t_plugobj *to, int inlet)
{
    if (!from || !to) {
        return 0;
    }
    // test if both inputs and outputs have the given inlet or outlet
    t_inlet *to_inlet = to->in_head;
    while (inlet) {
        if (!to_inlet) {
            return 0;
        }
        to_inlet = to_inlet->next;
        inlet--;
    }
    t_outlet *fr_outlet = from->out_head;
    while (outlet) {
        if (!fr_outlet) {
            return 0;
        }
        fr_outlet = fr_outlet->next;
        outlet--;
    }

    t_outlet_connection *connection = malloc(sizeof(t_outlet_connection));
    connection->target = to;
    connection->inlet = to_inlet;
    connection->prev = fr_outlet->tail;
    connection->next = NULL;
    if (fr_outlet->tail) {
        fr_outlet->tail->next = connection;
    } else {
        fr_outlet->head = connection;
    }
    fr_outlet->tail = connection;
    return 1;
}


#pragma mark - [patch]

t_patch* p_create_patch(void) {
    t_patch *patch = malloc(sizeof(t_patch));
    patch->inlets_head = patch->inlets_tail = NULL;
    patch->outlet_head = patch->outlet_tail = NULL;
    patch->instance_head = patch->instance_tail = NULL;
    patch->loadbang_head = patch->loadbang_tail = NULL;
    patch->name = NULL;
    patch->next = patch->prev = NULL;
    current_patch = patch;
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
    inlet_obj *iobj = idata;
    o_any(iobj->out, any);
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

static void patchobj_destroy(void *obj) {
    patch_obj* patch = obj;
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

#pragma mark - [inlet]

static void* inlet_create(int argc, char **argv) {
    inlet_obj *inlet = p_new(inlet_class);
    inlet->out = add_outlet(inlet, "incoming");
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

static void inlet_destroy(void* obj) {
    inlet_obj *inlet = obj;
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

#pragma mark - [outlet]

static void outlet_perform(void *obj, t_any any, void*idata) {
    outlet_obj* oobj = obj;
    o_any(oobj->out, any);
}

static void* outlet_create(int argc, char **argv) {
    outlet_obj *obj = p_new(outlet_class);
    t_inlet *outgoing = add_inlet(obj, "outgoing", NULL);
    p_inlet_add_any_fn(outgoing, outlet_perform);
    t_sym name = gen_sym("out");
    if (argc > 1) {
        name = gen_sym(argv[1]);
    }
    obj->out = add_outlet(current_patch, name);
    return obj;
}

static void outlet_destroy(void*obj) {
    outlet_obj *outlet = obj;
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

#pragma mark - Standard class library setup

void p_setup(void)
{
    outlet_class = p_create_plugclass(gen_sym("outlet"), sizeof(outlet_obj),
                                      outlet_create, outlet_destroy, NULL, NULL);
    inlet_class = p_create_plugclass(gen_sym("inlet"), sizeof(inlet_obj),
                                     inlet_create, inlet_destroy, NULL, NULL);
    patch_class = p_create_plugclass(gen_sym("patch"), sizeof(patch_obj),
                                     patchobj_create, patchobj_destroy, 0, 0);

    p_std_setup();
}

#pragma mark - Utilities

void p_bang(t_plugobj *obj, int inlet)
{
    
    t_inlet_fn *fns;
    t_inlet *inlet_ = obj->in_head;
    while (inlet--) {
        if (!inlet_) {
            return;
        }
        inlet_ = inlet_->next;
    }
    fns=inlet_->head;
    while (fns) {
        switch (fns->inlet_type) {
            case t_type_trigger:
                fns->inlet_trigger(obj, inlet_->data);
                return;
            case t_type_string:
                fns->inlet_string(obj, bang_sym, inlet_->data);
                return;
            case t_type_any:
                fns->inlet_any(obj, (t_any){t_type_trigger,0}, inlet_->data);
                return;
            default:
                break;
        }
        fns = fns->next;
    }
}

void p_setdrawdata(t_plugobj *obj, void *drawdata)
{
    obj->drawdata = drawdata;
}

