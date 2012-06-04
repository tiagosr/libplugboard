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

struct t_list {
    void *car, *cdr;
    int16_t car_type, cdr_type;
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
    t_inlet_fn **fns;
    t_inlet_fn *head, *tail;
    t_inlet *prev, *next;
    void *data;
};

struct t_outlet_connection {
    t_plugobj *target;
    t_inlet* inlet;
    t_outlet_connection *prev, *next;
};

struct t_outlet {
    t_sym name;
    // null-terminated list of connection pointers
    //t_outlet_connection **connections;
    t_outlet_connection *head, *tail;
    t_outlet *prev, *next;
};

struct t_plugobj {
    size_t inlet_count, outlet_count;
    char *obj_spec;
    t_plugclass *prototype;
    //t_inlet **inlets;
    t_inlet *in_head, *in_tail;
    //t_outlet **outlets;
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
void p_add_inlet(t_plugobj *obj, t_sym name, t_inlet_fn *fns, void *idata);
t_outlet *p_add_outlet(t_plugobj *obj, t_sym name);

t_plugobj* p_create_plugobj(const char* spec, void* drawdata);
int p_connect(t_plugobj *from, int outlet, t_plugobj *to, int inlet);
void p_destroy(t_plugobj* obj);

void p_bang(t_plugobj *obj, int inlet);
void p_setdrawdata(t_plugobj *obj, void *drawdata);

void p_setup(void);



#endif
