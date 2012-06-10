/*
 * Plugboard
 *
 * Patch-based programming language
 * Heavily influenced by Pure Data and it's kind, with a focus on interactivity
 * and control, exploring different usages apart from audio.
 * (c) 2012 Tiago Rezende
 *
 */

#ifndef livecode_plugboard_plugboard_h
#define livecode_plugboard_plugboard_h

#include <stdlib.h>

/*
 * general plugboard usage directives:
 *
 * externals: memory allocated by them for continuous usage should be dealt with
 * in the destructor. strings and lists allocated inside an inlet, if used for
 * an output, should be freed right after sending the object, as activation is
 * depth-first. inlets are not allowed to depend on sent pointers to strings and
 * lists persisting on their own.
 * by convention, like puredata externals, the leftmost inlet is a hot
 * (activating) inlet (with some notable exceptions, like [timer]), so create
 * your externals accordingly.
 * outlets' pointers should be stored in the object's structure for usage, but
 * should not be freed by the destructor function - the runtime takes care of
 * freeing them properly, same way with inlets.
 *
 * output messages: whenever possible, use the atomic types provided for output,
 * with the exception of t_sym, which should preferrably be used within a t_list
 * (as a message/keyword specifier). Pointer types should preferentially use the
 * t_any tagged pointer with an unique tag for each type you create. for the
 * moment, there's no tag registry, so take care not to clash type tags with other
 * externals.
 */

/*
 * type definitions
 */
typedef struct t_plugclass t_plugclass;
typedef struct t_plugobj t_plugobj;
typedef struct t_outlet t_outlet;
typedef struct t_outlet_connection t_outlet_connection;
typedef const char* t_sym;
typedef struct t_inlet t_inlet;
typedef struct t_inlet_fn t_inlet_fn;
typedef struct t_list t_list;
typedef struct t_any t_any;

typedef struct t_args {int argc; char **argv;} t_args;

/*
 * the creator function should always return an object created by p_new(class)
 * or nil if the creation process failed.
 */
typedef void* (*t_plugobj_creator_fn)(int argc, char **argv);
typedef void (*t_plugobj_destructor_fn)(void *obj);
typedef void (*t_plugobj_redraw_fn)(void *obj, t_any data,
                                    void *classdrawdata);

enum t_data_types {
    t_type_trigger = 0,
    t_type_int = 1,
    t_type_sym = 2,
    t_type_float = 3,
    t_type_string = 4,
    t_type_list = 5,
    t_type_any = 127,
    t_type_user = 128
};

enum t_connector_types {
    t_conn_normal = 0,
    t_conn_ref = 1,
    t_conn_dsp = 2,
    };


struct t_any {
    int type;
    union {
        int i_data;
        float f_data;
        t_sym sym;
        char *string;
        t_list* list;
        void *something;
    };
};

struct t_list {
    t_any first;
    t_list *rest;
};


typedef void(*t_inlet_trigger_fn)(void*obj, void*i_data);
typedef void(*t_inlet_int_fn)(void*obj, int ival, void*i_data);
typedef void(*t_inlet_float_fn)(void*obj, float fval, void*i_data);
typedef void(*t_inlet_sym_fn)(void*obj, t_sym sym, void*i_data);
typedef void(*t_inlet_string_fn)(void*obj, const char* string, void*i_data);
typedef void(*t_inlet_list_fn)(void*obj, t_list list, void*i_data);
typedef void(*t_inlet_any_fn)(void*obj, t_any any, void*i_data);

struct t_inlet_fn {
    int inlet_type;
    union {
        t_inlet_trigger_fn inlet_trigger;
        t_inlet_int_fn inlet_int;
        t_inlet_float_fn inlet_float;
        //t_inlet_sym_fn inlet_sym;
        t_inlet_string_fn inlet_string;
        t_inlet_list_fn inlet_list;
        t_inlet_any_fn inlet_any;
    };
    t_inlet_fn *next;
};
struct t_inlet {
    t_sym name;
    t_inlet_fn *head, *tail;
    t_inlet *prev, *next;
    void *data;
    int tag;
};

struct t_outlet_connection {
    t_plugobj *target;
    t_inlet* inlet;
    t_outlet_connection *prev, *next;
};

struct t_outlet {
    t_sym name;
    t_outlet_connection *head, *tail;
    t_outlet *prev, *next;
    int tag;
};

struct t_plugobj {
    size_t inlet_count, outlet_count;
    char *obj_spec;
    t_plugclass *prototype;
    t_inlet *in_head, *in_tail;
    t_outlet *out_head, *out_tail;
    void *drawdata;
};

struct t_plugclass {
    t_sym classname;
    t_plugobj_creator_fn creator;
    t_plugobj_destructor_fn destructor;
    t_plugobj_redraw_fn redraw;
    void *classdata;
    size_t obj_size;
};
typedef struct t_patch t_patch;
typedef struct inlet_obj inlet_obj;
typedef struct outlet_obj outlet_obj;
typedef struct patch_obj patch_obj;
typedef struct loadbang_obj loadbang_obj;

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
    t_outlet *out;
};

struct outlet_obj {
    t_plugobj base;
    size_t pos;
    t_patch *owner;
    t_outlet *out;
    outlet_obj *prev, *next;
};

/*
 * finds or creates a single instance of a given string for simpler symbol treatment
 */
t_sym gen_sym(const char *s);
t_sym find_sym(const char *s);


/*
 * splits a string into arguments for object creation.
 * you must free argv[0] and argv when done with it.
 */
t_args p_separate_args(char *s);

void p_outlet_bang(t_outlet *outlet);
void p_outlet_send_int(t_outlet *outlet, int data);
void p_outlet_send_float(t_outlet *outlet, float data);
void p_outlet_send_string(t_outlet *outlet, const char* data);
void p_outlet_send_list(t_outlet *outlet, t_list data);
void p_outlet_send_any(t_outlet *outlet, t_any any);

void p_send_redraw(t_plugobj *obj, t_any data);

t_list p_list_cons(int16_t a_type, void* a_data, int16_t b_type, void* b_data);

t_plugclass* p_create_plugclass(t_sym symbol, size_t object_size,
                                t_plugobj_creator_fn creator,
                                t_plugobj_destructor_fn destructor,
                                t_plugobj_redraw_fn redraw,
                                void *classdata);
void p_create_class_alias(t_plugclass *ref, t_sym alias);
/*
 * creates a new plugobject based on the specs of the plugclass.
 * to be used in object creator functions.
 * returns a structure with the size specified by spec->object_size
 * (t_plugobj + object specifics), whose t_plugobj section is filled
 * according to the spec.
 */
void* p_new(t_plugclass *spec);
t_inlet *p_add_inlet(t_plugobj *obj, t_sym name, void *idata);
void p_inlet_add_trigger_fn(t_inlet *inlet, t_inlet_trigger_fn fn);
void p_inlet_add_int_fn(t_inlet *inlet, t_inlet_int_fn fn);
void p_inlet_add_float_fn(t_inlet *inlet, t_inlet_float_fn fn);
void p_inlet_add_string_fn(t_inlet *inlet, t_inlet_string_fn fn);
void p_inlet_add_list_fn(t_inlet *inlet, t_inlet_list_fn fn);
void p_inlet_add_any_fn(t_inlet *inlet, t_inlet_any_fn fn);

void p_inlet_set_tag(t_inlet *inlet, int tag);
int p_inlet_get_tag(t_inlet *inlet);

t_outlet *p_add_outlet(t_plugobj *obj, t_sym name);

void p_outlet_set_tag(t_outlet *outlet, int tag);
int p_outlet_get_tag(t_outlet *outlet);

t_plugobj* p_create_plugobj(const char* spec, void* drawdata);
int p_connect(t_plugobj *from, int outlet, t_plugobj *to, int inlet);
void p_destroy(t_plugobj* obj);

void p_bang(t_plugobj *obj, int inlet);
void p_setdrawdata(t_plugobj *obj, void *drawdata);


t_patch *p_create_patch(void);
void p_patch_set_current(t_patch *patch);
void p_patch_current_add_subpatch(t_patch *subpatch, t_sym name);


void p_setup(void);



#endif
