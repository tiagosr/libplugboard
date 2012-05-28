//
//  plugboard-lib-std-debug.h
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/5/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#ifndef livecode_plugboard_plugboard_lib_std_debug_h
#define livecode_plugboard_plugboard_lib_std_debug_h
#include "plugboard-lib-std.h"

void p_print_set_hook(void (*print_hook)(const char*string));

void p_std_debug_setup(void);


#endif
