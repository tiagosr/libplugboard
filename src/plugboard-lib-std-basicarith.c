//
//  plugboard-lib-std-basicarith.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/13/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std-basicarith.h"
#include "plugboard-utils.h"
#include <math.h>
#include <string.h>

static t_plugclass *i_add_class, *i_sub_class, *i_mul_class, *i_div_class, 
    *i_shift_left_class, *i_shift_right_class, *i_compare_class;

typedef struct i_offset_obj i_offset_obj;
struct i_offset_obj {
    t_plugobj base;
    int result;
    int offset;
    t_outlet *out;
};

#pragma mark - [(+,-,*,/,<<,>>)i]

static void i_add_perform(void *obj, int num, void *idata)
{
    i_offset_obj *iobj = obj;
    iobj->result = num + iobj->offset;
    o_int(iobj->out, iobj->result);
}

static void i_sub_perform(void *obj, int num, void *idata)
{
    i_offset_obj *iobj = obj;
    iobj->result = num - iobj->offset;
    o_int(iobj->out, iobj->result);
}

static void i_set_offset(void *obj, int num, void *idata)
{
    ((i_offset_obj *)obj)->offset = num;
}

static void i_mul_perform(void *obj, int num, void *idata)
{
    i_offset_obj *iobj = (i_offset_obj *)obj;
    iobj->result = num * iobj->offset;
    o_int(iobj->out, iobj->result);
}

static void i_div_setfactor(void *obj, int num, void *idata)
{
    if (num == 0) {
        ((i_offset_obj *)obj)->offset = 0;
    } else {
        ((i_offset_obj *)obj)->offset = (1.0/(float)num);
    }
}

static void i_shl_perform(void *obj, int num, void *idata)
{
    i_offset_obj *iobj = obj;
    iobj->result = num << iobj->offset;
    o_int(iobj->out, iobj->result);
}

static void i_shr_perform(void *obj, int num, void *idata)
{
    i_offset_obj *iobj = obj;
    iobj->result = num >> iobj->offset;
    o_int(iobj->out, iobj->result);
}

static void i_shift_setoffset(void *obj, int num, void *idata)
{
    ((i_offset_obj *)obj)->offset = num>0? (num<(sizeof(int)*8)? num : sizeof(int)*8):0;
}

static void i_send_result(void *obj, void *idata)
{
    o_int(((i_offset_obj *)obj)->out, ((i_offset_obj *)obj)->result);
}

static void* i_op_create(t_plugclass *pclass, int argc, char **argv, t_inlet_int_fn perform)
{
    i_offset_obj* iobj = p_new(pclass);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    t_inlet *value = add_inlet(iobj,"value", NULL);
    p_inlet_add_int_fn(value, perform);
    p_inlet_add_trigger_fn(value, i_send_result);
    t_inlet *offset = add_inlet(iobj,"offset", NULL);
    p_inlet_add_int_fn(offset, i_set_offset);
    iobj->out = add_outlet(iobj,"result");
    return iobj;
    
}
static void* i_add_create(int argc, char **argv)
{
    return i_op_create(i_add_class, argc, argv, i_add_perform);
}

static void* i_sub_create(int argc, char **argv)
{
    return i_op_create(i_sub_class, argc, argv, i_sub_perform);
}

static void* i_mul_create(int argc, char **argv)
{
    return i_op_create(i_mul_class, argc, argv, i_mul_perform);
}

static void* i_div_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_div_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    t_inlet *value = add_inlet(iobj,"value", NULL);
    p_inlet_add_int_fn(value, i_mul_perform);
    p_inlet_add_trigger_fn(value, i_send_result);
    t_inlet *offset = add_inlet(iobj,"ratio", NULL);
    p_inlet_add_int_fn(offset, i_div_setfactor);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}

static void* i_shl_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_shift_left_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    t_inlet *value = add_inlet(iobj,"value", NULL);
    p_inlet_add_int_fn(value, i_shl_perform);
    p_inlet_add_trigger_fn(value, i_send_result);
    t_inlet *offset = add_inlet(iobj,"offset", NULL);
    p_inlet_add_int_fn(offset, i_shift_setoffset);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}
static void* i_shr_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_shift_right_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    t_inlet *value = add_inlet(iobj,"value", NULL);
    p_inlet_add_int_fn(value, i_shr_perform);
    p_inlet_add_trigger_fn(value, i_send_result);
    t_inlet *offset = add_inlet(iobj,"offset", NULL);
    p_inlet_add_int_fn(offset, i_shift_setoffset);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}

#pragma mark - [(=,!=,<,>,<=,>=)i]

static void i_compare_equals(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(iobj->offset==val) {
		o_bang(iobj->out);
	}
}
static void i_compare_not_equal(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(iobj->offset!=val) {
		o_bang(iobj->out);
	}
}

static void i_compare_less_than(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(val < iobj->offset) {
		o_bang(iobj->out);
	}
}

static void i_compare_greater_than(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(val>iobj->offset) {
		o_bang(iobj->out);
	}
}
static void i_compare_less_than_or_equal(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(val <= iobj->offset) {
		o_bang(iobj->out);
	}
}

static void i_compare_greater_than_or_equal(void*obj, int val, void*idata)
{
	i_offset_obj* iobj = obj;
	if(val>=iobj->offset) {
		o_bang(iobj->out);
	}
}

static void* i_compare_create(int argc, char** argv)
{
	i_offset_obj *obj = p_new(i_compare_class);
	obj->out = add_outlet(obj,"true");
	t_inlet * a = add_inlet(obj, "a", NULL);

	if (strcmp(argv[0],"=i")==0) {
		p_inlet_add_int_fn(a, i_compare_equals);
	} else if (strcmp(argv[0],"!=i")==0) {
		p_inlet_add_int_fn(a, i_compare_not_equal);
	} else 
	if (strcmp(argv[0],"<i")==0) {
        p_inlet_add_int_fn(a, i_compare_less_than);
	} else 
	if (strcmp(argv[0],"<=i")==0) {
        p_inlet_add_int_fn(a, i_compare_less_than_or_equal);
	} else if (strcmp(argv[0],">i")==0) {
        p_inlet_add_int_fn(a, i_compare_greater_than);
	} else if (strcmp(argv[0],"=i")==0) {
        p_inlet_add_int_fn(a, i_compare_greater_than_or_equal);

	}
	return obj;
}

/// logic operators

// [&&]

// end [&&]

// [||]

// end [||]

// [!]

// end [!]

/// end logic operators

/// bitwise operators

/// end bitwise operators

// [iclip]

// end [iclip]

// [imax]

// end [imax]

// [imin]

// end [imin]

#pragma mark - int arithmetics lib setup

static void p_std_basicarith_int_setup(void)
{
    i_add_class = p_create_plugclass(gen_sym("+i"), sizeof(i_offset_obj), 
                                     i_add_create, NULL, NULL, NULL);
    i_sub_class = p_create_plugclass(gen_sym("-i"), sizeof(i_offset_obj), 
                                     i_sub_create, NULL, NULL, NULL);
    i_mul_class = p_create_plugclass(gen_sym("*i"), sizeof(i_offset_obj), 
                                     i_mul_create, NULL, NULL, NULL);
    i_div_class = p_create_plugclass(gen_sym("/i"), sizeof(i_offset_obj), 
                                     i_div_create, NULL, NULL, NULL);
    i_shift_left_class = p_create_plugclass(gen_sym("<<"), sizeof(i_offset_obj),
                                            i_shl_create, NULL, NULL, NULL);
    i_shift_right_class = p_create_plugclass(gen_sym(">>"), sizeof(i_offset_obj),
                                             i_shr_create, NULL, NULL, NULL);
    i_compare_class = p_create_plugclass(gen_sym("=i"),sizeof(i_offset_obj),
                                             i_compare_create, NULL, NULL, NULL);
    p_create_class_alias(i_compare_class, gen_sym("!=i"));
    p_create_class_alias(i_compare_class, gen_sym(">=i"));
    p_create_class_alias(i_compare_class, gen_sym("<=i"));
    p_create_class_alias(i_compare_class, gen_sym(">i"));
    p_create_class_alias(i_compare_class, gen_sym("<i"));
}

#pragma mark - [(+,-,*,/)]

static t_plugclass *f_add_class, *f_sub_class, *f_mul_class, *f_div_class, *f_compare_class, *f_moses_class;

typedef struct f_offset_obj {
    t_plugobj base;
    float result;
    float offset;
    t_outlet *out;
} f_offset_obj;

static void f_send_result(void *obj, void* idata)
{
    o_float(((f_offset_obj *)obj)->out, ((f_offset_obj *)obj)->result);
}

static void f_set_offset(void *obj, float val, void* idata)
{
    ((f_offset_obj *)obj)->offset = val;
}

static void f_add_perform(void *obj, float val, void* idata)
{
    f_offset_obj *fobj = obj;
    fobj->result = val + fobj->offset;
    o_float(fobj->out, fobj->result);
}

static void f_sub_perform(void *obj, float val, void* idata)
{
    f_offset_obj *fobj = obj;
    fobj->result = val - fobj->offset;
    o_float(fobj->out, fobj->result);
}

static void f_mul_perform(void *obj, float val, void* idata)
{
    f_offset_obj *fobj = obj;
    fobj->result = val * fobj->offset;
    o_float(fobj->out, fobj->result);
}

static void f_div_setratio(void *obj, float divisor, void* idata)
{
    ((f_offset_obj *)obj)->offset = (divisor==0)?0:(1.0/divisor);
}

static void f_sqrt_perform(void *obj, float val, void *idata)
{
    o_float(((f_offset_obj *)obj)->out, sqrtf(val));
}

static void f_isgt_perform(void *obj, float val, void *idata)
{
    f_offset_obj *fobj;
    fobj->result = val;
    if (val > fobj->offset) {
        o_bang(fobj->out);
    }
}

static void f_islt_perform(void *obj, float val, void *idata)
{
    f_offset_obj *fobj;
    fobj->result = val;
    if (val < fobj->offset) {
        o_bang(fobj->out);
    }
}

static void* f_op_create(t_plugclass *pclass, int argc, char **argv, t_inlet_float_fn perform)
{
    f_offset_obj *obj = p_new(pclass);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    t_inlet *value = add_inlet(obj,"value", NULL);
    p_inlet_add_float_fn(value, perform);
    p_inlet_add_trigger_fn(value, f_send_result);
    t_inlet *offset = add_inlet(obj,"offset", NULL);
    p_inlet_add_float_fn(offset, f_set_offset);
    
    add_outlet(obj, "out");
    return obj;
}

static void* f_add_create(int argc, char **argv)
{
    return f_op_create(f_add_class, argc, argv, f_add_perform);
}

static void* f_sub_create(int argc, char **argv)
{
    return f_op_create(f_sub_class, argc, argv, f_sub_perform);
}

static void* f_mul_create(int argc, char **argv)
{
    return f_op_create(f_mul_class, argc, argv, f_mul_perform);
}

static void* f_div_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_div_class);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    t_inlet *value = add_inlet(obj,"value", NULL);
    p_inlet_add_float_fn(value, f_mul_perform);
    p_inlet_add_trigger_fn(value, f_send_result);
    t_inlet *offset = add_inlet(obj,"ratio", NULL);
    p_inlet_add_float_fn(offset, f_div_setratio);
    add_outlet(obj, "out");
    return obj;
}

static void* f_cmp_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_compare_class);
    if (argc > 1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    t_inlet *value = add_inlet(obj,"a", NULL);
    t_inlet *offset = add_inlet(obj,"b", NULL);
    p_inlet_add_float_fn(offset, f_set_offset);
    
    add_outlet(obj, "out");

    if (strcmp("<", argv[0])==0) {
        p_inlet_add_float_fn(value, f_islt_perform);
    } else {
        p_inlet_add_float_fn(value, f_isgt_perform);
    }
    add_outlet(obj, "true");
    return obj;
}


#pragma mark - [moses]
typedef struct moses_obj {
    t_plugobj base;
    float cut, result;
    t_outlet *less, *more;
} moses_obj;

static void f_moses_send(void *obj, void *idata)
{
    moses_obj *fobj = obj;
    if (fobj->result < fobj->cut) {
        o_float(fobj->less, fobj->result);
    } else {
        o_float(fobj->more, fobj->result);
    }
}


static void f_moses_perform(void *obj, float val, void *idata)
{
    moses_obj *fobj = obj;
    fobj->result = val;
    if (val < fobj->cut) {
        o_float(fobj->less, val);
    } else {
        o_float(fobj->more, val);
    }
}

static void *f_moses_create(int argc, char **argv)
{
    moses_obj *obj = p_new(f_moses_class);
    if (argc > 1) {
        obj->cut = atof(argv[1]);
    } else {
        obj->cut = 0.0;
    }    
    add_outlet(obj, "out");
    obj->less = add_outlet(obj, "less-than");
    obj->more = add_outlet(obj, "greater-than");
    t_inlet *value = add_inlet(obj,"in", NULL);
    p_inlet_add_float_fn(value, f_moses_perform);
    t_inlet *offset = add_inlet(obj,"compare", NULL);
    p_inlet_add_float_fn(offset, f_moses_send);
    
    return obj;
}
// end [moses]

// [pow]

// end [pow]

// [sqrt]

// end [sqrt]

// [clip]

// end [clip]

// [max]

// end [max]

// [min]

// end [min]

#pragma - float arithmetics library setup

static void p_std_basicarith_float_setup(void)
{
    f_add_class = p_create_plugclass(gen_sym("+"), sizeof(f_offset_obj),
                                     f_add_create, NULL, NULL, NULL);
    f_sub_class = p_create_plugclass(gen_sym("-"), sizeof(f_offset_obj),
                                     f_sub_create, NULL, NULL, NULL);
    f_mul_class = p_create_plugclass(gen_sym("*"), sizeof(f_offset_obj),
                                     f_mul_create, NULL, NULL, NULL);
    f_div_class = p_create_plugclass(gen_sym("/"), sizeof(f_offset_obj),
                                     f_div_create, NULL, NULL, NULL);
    f_compare_class = p_create_plugclass(gen_sym("<"), sizeof(f_offset_obj), 
                                         f_cmp_create, NULL, NULL, NULL);
    p_create_class_alias(f_compare_class, gen_sym(">"));
    f_moses_class = p_create_plugclass(gen_sym("moses"), sizeof(moses_obj),
                                       f_moses_create, NULL, NULL, NULL);
}

#pragma - arithmetics library setup

void p_std_basicarith_setup(void)
{
    p_std_basicarith_int_setup();
    p_std_basicarith_float_setup();
}

