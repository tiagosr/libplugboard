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

static void* i_add_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_add_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    inlet_fn_list(fns0)
        i_fn_int(i_add_perform),
        i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
        i_fn_int(i_set_offset),
    inlet_fn_list_end
    
    add_inlet(iobj,"value", fns0, NULL);
    add_inlet(iobj,"offset", fns1, NULL);
    iobj->out =add_outlet(iobj,"result");
    return iobj;
}

static void* i_sub_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_sub_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    inlet_fn_list(fns0)
        i_fn_int(i_sub_perform),
        i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
        i_fn_int(i_set_offset),
    inlet_fn_list_end
    
    add_inlet(iobj, "value", fns0, NULL);
    add_inlet(iobj, "offset", fns1, NULL);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}

static void* i_mul_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_mul_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    inlet_fn_list(fns0)
        i_fn_int(i_mul_perform),
        i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
        i_fn_int(i_set_offset),
    inlet_fn_list_end
    
    add_inlet(iobj, "value", fns0, NULL);
    add_inlet(iobj, "factor", fns1, NULL);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}

static void* i_div_create(int argc, char **argv)
{
    i_offset_obj* iobj = p_new(i_div_class);
    if (argc>1) {
        iobj->offset = atoi(argv[1]);
    } else {
        iobj->offset = 0;
    }
    inlet_fn_list(fns0)
        i_fn_int(i_mul_perform),
        i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
        i_fn_int(i_div_setfactor),
    inlet_fn_list_end
    
    add_inlet(iobj, "value", fns0, NULL);
    add_inlet(iobj, "ratio", fns1, NULL);
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
    inlet_fn_list(fns0)
    i_fn_int(i_shl_perform),
    i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
    i_fn_int(i_shift_setoffset),
    inlet_fn_list_end
    
    add_inlet(iobj, "value", fns0, NULL);
    add_inlet(iobj, "ratio", fns1, NULL);
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
    inlet_fn_list(fns0)
    i_fn_int(i_shr_perform),
    i_fn_trigger(i_send_result),
    inlet_fn_list_end
    inlet_fn_list(fns1)
    i_fn_int(i_shift_setoffset),
    inlet_fn_list_end
    
    add_inlet(iobj, "value", fns0, NULL);
    add_inlet(iobj, "ratio", fns1, NULL);
    iobj->out = add_outlet(iobj, "result");
    return iobj;
}

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
	
	inlet_fn_list(fns_equal)
    i_fn_int(i_compare_equals),
    inlet_fn_list_end
    
	inlet_fn_list(fns_not_equal)
    i_fn_int(i_compare_not_equal),
    inlet_fn_list_end
    
	inlet_fn_list(fns_lt)
    i_fn_int(i_compare_less_than),
    inlet_fn_list_end
    
	inlet_fn_list(fns_lte)
    i_fn_int(i_compare_less_than_or_equal),
    inlet_fn_list_end
    
	inlet_fn_list(fns_gt)
    i_fn_int(i_compare_greater_than),
    inlet_fn_list_end
    
	inlet_fn_list(fns_gte)
    i_fn_int(i_compare_greater_than_or_equal),
    inlet_fn_list_end
                
	if (strcmp(argv[0],"=")==0) {
		add_inlet(obj, "a", fns_equal, NULL);
	} else if (strcmp(argv[0],"!=")==0) {
		add_inlet(obj, "a", fns_not_equal, NULL);
	} else 
	if (strcmp(argv[0],"<")==0) {
		add_inlet(obj, "a", fns_lt, NULL);
	} else 
	if (strcmp(argv[0],"<=")==0) {
		add_inlet(obj, "a", fns_lte, NULL);
	} else if (strcmp(argv[0],">")==0) {
		add_inlet(obj, "a", fns_gt, NULL);
	} else if (strcmp(argv[0],"=")==0) {
		add_inlet(obj, "a", fns_gte, NULL);
	}
	return obj;
}



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
    i_compare_class = p_create_plugclass(gen_sym("="),sizeof(i_offset_obj),
                                             i_compare_create, NULL, NULL, NULL);
}

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

static void f_moses_send(void *obj, void *idata)
{
    f_offset_obj *fobj = obj;
    if (fobj->result < fobj->offset) {
        o_float(fobj->out, fobj->result);
    } else {
        o_float(fobj->out, fobj->result);
    }
}

typedef struct moses_obj {
    t_plugobj base;
    float cut, result;
    t_outlet *less, *more;
} moses_obj;

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


static void* f_add_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_add_class);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    inlet_fn_list(fn0)
        i_fn_float(f_add_perform),
        i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_fn_list(fn1)
        i_fn_float(f_set_offset),
    inlet_fn_list_end

    add_inlet(obj, "in", fn0, NULL);
    add_inlet(obj, "offset", fn1, NULL);
    add_outlet(obj, "out");
    return obj;
}

static void* f_sub_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_add_class);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    inlet_fn_list(fn0)
    i_fn_float(f_sub_perform),
    i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_fn_list(fn1)
    i_fn_float(f_set_offset),
    inlet_fn_list_end
    
    add_inlet(obj, "in", fn0, NULL);
    add_inlet(obj, "offset", fn1, NULL);
    add_outlet(obj, "out");
    return obj;
}

static void* f_mul_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_mul_class);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    inlet_fn_list(fn0)
    i_fn_float(f_mul_perform),
    i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_fn_list(fn1)
    i_fn_float(f_set_offset),
    inlet_fn_list_end
    
    add_inlet(obj, "in", fn0, NULL);
    add_inlet(obj, "factor", fn1, NULL);
    add_outlet(obj, "out");
    return obj;
}

static void* f_div_create(int argc, char **argv)
{
    f_offset_obj *obj = p_new(f_add_class);
    if (argc>1) {
        obj->offset = atof(argv[1]);
    } else {
        obj->offset = 0.0;
    }
    inlet_fn_list(fn0)
    i_fn_float(f_mul_perform),
    i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_float_fn(fn1,f_div_setratio);
    
    add_inlet(obj, "in", fn0, NULL);
    add_inlet(obj, "ratio", fn1, NULL);
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
    inlet_fn_list(fngt)
        i_fn_float(f_isgt_perform),
        i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_fn_list(fnlt)
        i_fn_float(f_islt_perform),
        i_fn_trigger(f_send_result),
    inlet_fn_list_end
    inlet_float_fn(fn1,f_set_offset);
    if (strcmp("<", argv[0])==0) {
        add_inlet(obj, "in", fnlt, NULL);
    } else {
        add_inlet(obj, "in", fngt, NULL);
    }
    add_inlet(obj, "compare", fn1, NULL);
    add_outlet(obj, "true");
    return obj;
}

static void *f_moses_create(int argc, char **argv)
{
    moses_obj *obj = p_new(f_moses_class);
    if (argc > 1) {
        obj->cut = atof(argv[1]);
    } else {
        obj->cut = 0.0;
    }
    inlet_float_fn(fn0, f_moses_perform);
    inlet_float_fn(fn1, f_set_offset);
    
    add_inlet(obj, "in", fn0, NULL);
    add_inlet(obj, "compare", fn1, NULL);
    obj->less = add_outlet(obj, "less-than");
    obj->more = add_outlet(obj, "greater-than");
    return obj;
}

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


void p_std_basicarith_setup(void)
{
    p_std_basicarith_int_setup();
    p_std_basicarith_float_setup();
}

