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
void p_std_setup(void);
typedef struct t_plugclass_name {
    t_sym name; t_plugclass *ref;
} t_plugclass_name;

//static unsigned stack_depth = 0;
static int symbol_pool_size = -1, symbol_pool_occupied = 0;
static t_sym * symbol_pool = NULL;

static int class_pool_size = -1, class_pool_occupied = 0;
static t_plugclass_name *class_pool = NULL;
static t_sym bang_sym = NULL;

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

/*
 * outlet sentry functions
 */


void p_outlet_send_int(t_plugobj *obj, size_t outlet, int data)
{
    t_outlet_connection **current = obj->outlets[outlet]->connections;
    while (current[0]) {
        /* try sending to the given inlet as one of the basic types,
         * in the following order:
         * int, float, string, any, trigger
         * once a suitable inlet function is found, send and proceed
         * to the next inlet
         */
        t_inlet_fn **inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_int) {
                inlet[0]->inlet_int(current[0]->target,data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_float) {
                inlet[0]->inlet_float(current[0]->target,data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_any) {
                send_int_as_any(current[0]->target,inlet[0],data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_string) {
                send_int_as_string(current[0]->target,inlet[0],data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_trigger) {
                inlet[0]->inlet_trigger(current[0]->target,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
    sent:
        current++;
    }
}

void p_outlet_send_float(t_plugobj *obj, size_t outlet, float data)
{
    t_outlet_connection **current = obj->outlets[outlet]->connections;
    while (current[0]) {
        t_inlet_fn **inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_float) {
                inlet[0]->inlet_float(current[0]->target,data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_any) {
                send_float_as_any(current[0]->target,inlet[0],data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_string) {
                send_float_as_string(current[0]->target,inlet[0],data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_int) {
                inlet[0]->inlet_int(current[0]->target,data,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_trigger) {
                inlet[0]->inlet_trigger(current[0]->target,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
    sent:
        current++;
    }
}


void p_outlet_send_string(t_plugobj *obj, size_t outlet, const char* string)
{
    t_outlet_connection **current = obj->outlets[outlet]->connections;
    while (current[0]) {
        t_inlet_fn **inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_string) {
                inlet[0]->inlet_string(current[0]->target,string,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        t_any any_sym = {t_type_sym, .string=(char *)string};
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_any) {
                inlet[0]->inlet_any(current[0]->target,any_sym,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_trigger) {
                inlet[0]->inlet_trigger(current[0]->target,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
    sent:
        current++;
    }
}

void p_outlet_send_any(t_plugobj *obj, size_t outlet, t_any any)
{
    t_outlet_connection **current = obj->outlets[outlet]->connections;
    while (current[0]) {
        t_inlet_fn **inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_string) {
                inlet[0]->inlet_any(current[0]->target,any,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (any.type == inlet[0]->inlet_type) {
                switch (any.type) {
                    case t_type_int:
                        inlet[0]->inlet_int(current[0]->target,any.i_data,current[0]->inlet.data);
                        goto sent;
                    case t_type_float:
                        inlet[0]->inlet_float(current[0]->target,any.f_data,current[0]->inlet.data);
                        goto sent;
                    case t_type_string:
                        inlet[0]->inlet_string(current[0]->target,any.string,current[0]->inlet.data);
                        goto sent;
                    case t_type_list:
                        inlet[0]->inlet_list(current[0]->target,*any.list,current[0]->inlet.data);
                        goto sent;
                }
            }
            inlet++;
        }
        inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            if (inlet[0]->inlet_type == t_type_trigger) {
                inlet[0]->inlet_trigger(current[0]->target,current[0]->inlet.data);
                goto sent;
            }
            inlet++;
        }
    sent:
        current++;
    }
}

void p_outlet_bang(t_plugobj *obj, size_t outlet)
{
    t_outlet_connection **current = obj->outlets[outlet]->connections;
    while (current[0]) {
        t_inlet_fn **inlet = current[0]->inlet.fns;
        while (inlet[0]) {
            switch (inlet[0]->inlet_type) {
                case t_type_trigger:
                    inlet[0]->inlet_trigger(current[0]->target,
                                                    current[0]->inlet.data);
                    break;
                /*
                case t_type_sym:
                    inlet[0]->inlet_sym(current[0]->target,bang_sym,
                                                current[0]->inlet.data);
                    break;
                */
                case t_type_string:
                    inlet[0]->inlet_string(current[0]->target,bang_sym,
                                                   current[0]->inlet.data);
                    break;
                default:
                    inlet[0]->inlet_any(current[0]->target,(t_any){t_type_trigger,0},
                                                current[0]->inlet.data);
            }
            inlet++;
        }
        current++;
    }
}

void p_send_redraw(t_plugobj *obj, t_any data)
{
    if (obj && obj->prototype->redraw) {
        obj->prototype->redraw(obj,data,obj->prototype->classdata);
    }
}

/*
 * inlet creation
 */
void p_add_inlet(t_plugobj *obj, t_sym name, t_inlet_fn *fns, void *idata)
{
    int fn_len = 0;
    t_inlet *inlet = malloc(sizeof(t_inlet));
    inlet->name = name;
    inlet->data = idata;
    inlet->fns = malloc(sizeof(t_inlet_fn*)*2);
    while (fns[0].inlet_any) {
        t_inlet_fn *inlet_fn = malloc(sizeof(t_inlet_fn));
        inlet_fn->inlet_type = fns[0].inlet_type;
        inlet_fn->inlet_any = fns[0].inlet_any;
        inlet->fns[fn_len] = inlet_fn;
        fn_len++;
        inlet->fns = realloc(inlet->fns, sizeof(t_inlet_fn*)+(fn_len+1));
        fns++;
    }
    inlet->fns[fn_len] = 0;
    obj->inlets = realloc(obj->inlets, sizeof(t_inlet)*(obj->inlet_count+2));
    obj->inlets[obj->inlet_count] = inlet;
    obj->inlets[obj->inlet_count+1] = 0;
    obj->inlet_count++;
}


/*
 * outlet creation
 */

void p_add_outlet(t_plugobj *obj, t_sym name)
{
    t_outlet *outlet = malloc(sizeof(outlet));
    outlet->name = name;
    outlet->connections = malloc(sizeof(t_outlet_connection*));
    outlet->connections[0] = NULL;
    obj->outlets = realloc(obj->outlets, sizeof(t_outlet*)*(obj->outlet_count+2));
    obj->outlets[obj->outlet_count] = outlet;
    obj->outlets[obj->outlet_count+1] = 0;
    obj->outlet_count++;
}



/*
 * embed-side utilities
 */
t_list p_list_cons(int16_t a_type, void* a_data, int16_t b_type, void* b_data)
{
    t_list temp = {a_data, b_data, a_type, b_type};
    return temp;
}


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


void* p_new(t_plugclass *class_spec)
{
    t_plugobj* temp = malloc(class_spec->obj_size);
    temp->prototype = class_spec;
    temp->inlets = malloc(sizeof(t_inlet_fn *));
    temp->inlets[0] = NULL;
    temp->outlets = malloc(sizeof(t_outlet *));
    temp->outlets[0] = NULL;
    return temp;
}

struct arg_list {
    char *start;
    struct arg_list *next;
};
static struct arg_list* separate_args_rec(char *string, int nesting, int in_string, int ignore)
{
    if (!string[0]) {
        return NULL;
    } else if (ignore) {
        return separate_args_rec(string+1, nesting, in_string, ignore-1);
    } else if (string[0]=='\\') {
        return separate_args_rec(string+1, nesting, in_string, ignore+1);
    } else if (string[0]=='"') {
        if (in_string) {
            string[0] = 0;
            return separate_args_rec(string+1, nesting, 0, ignore);
        } else {
            string[0] = 0;
            if (!nesting) {
                struct arg_list* args = malloc(sizeof(struct arg_list));
                args->start = string+1;
                args->next = separate_args_rec(string+1, nesting, 1, ignore);
                return args;
            } else {
                return separate_args_rec(string+1, nesting, 1, ignore);
            }
        }
    } else if ((string[0]==' ')||(string[0]=='\n')||(string[0]=='\t')) {
        if (nesting||in_string||ignore) {
            return separate_args_rec(string+1, nesting, in_string, ignore);
        } else {
            struct arg_list* rec = separate_args_rec(string+1, nesting, 1, ignore);
            if (!rec || rec->start>string+2) {
                string[0]=0;
                struct arg_list* args = malloc(sizeof(struct arg_list));
                args->start = string+1;
                args->next = rec;
                return args;
            } else {
                return rec;
            }
        }
    } else {
        return separate_args_rec(string+1, nesting, in_string, ignore);
    }
}

t_args p_separate_args(char *string)
{
    int argc = 0;
    struct arg_list *piter = malloc(sizeof(struct arg_list));
    struct arg_list *list = separate_args_rec(string, 0, 0, 0), *iter = piter;
    iter->start = string;
    iter->next = list;
    while (iter!=NULL) {
        argc++;
        iter=iter->next;
    }
    char **argv = malloc(sizeof(char*)*argc);
    iter = piter;
    argc = 0;
    while (iter!=NULL) {
        void *prev = iter;
        argv[argc]=iter->start;
        argc++;
        iter=iter->next;
        free(prev);
    }
    return (t_args){argc, argv};
}

t_plugobj* p_create_plugobj(const char* spec)
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
    return res;
}

void p_destroy(t_plugobj *obj) {
    obj->prototype->destructor(obj);
    free(obj);
}

/*
 * connecting stuff up
 */

int p_connect(t_plugobj *from, int outlet, t_plugobj *to, int inlet)
{
    if (!from || !to) {
        return 0;
    }
    // test if both inputs and outputs have the given inlet or outlet
    for (int o=0; o<=outlet; o++) {
        if (from->outlets[o] == 0) {
            return 0;
        }
    }
    for (int i=0; i<=inlet; i++) {
        if (to->inlets[i] == 0) {
            return 0;
        }
    }
    t_outlet_connection **connections = from->outlets[outlet]->connections;
    t_outlet_connection *connection = malloc(sizeof(t_outlet_connection));
    connection->target = to;
    connection->inlet = *to->inlets[inlet];
    size_t conn_size = 0;
    while (connections[conn_size]) {
        conn_size++;
    }
    from->outlets[outlet]->connections = realloc(from->outlets[outlet]->connections, sizeof(t_outlet_connection*)*(conn_size+1));
    connections[conn_size+1] = 0;
    connections[conn_size] = connection;
    return 1;
}

void p_setup(void)
{
    p_std_setup();
}

void p_bang(t_plugobj *obj, int inlet)
{
    t_inlet_fn **fns = obj->inlets[inlet]->fns;
    while (fns[0]) {
        switch (fns[0]->inlet_type) {
            case t_type_trigger:
                fns[0]->inlet_trigger(obj, obj->inlets[inlet]->data);
                return;
            case t_type_string:
                fns[0]->inlet_string(obj, bang_sym, obj->inlets[inlet]->data);
                return;
            case t_type_any:
                fns[0]->inlet_any(obj, (t_any){t_type_trigger,0}, obj->inlets[inlet]->data);
                return;
            default:
                break;
        }
    }
}

