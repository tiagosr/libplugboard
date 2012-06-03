//
//  plugboard-lib-std.c
//  livecode-plugboard
//
//  Created by Tiago Rezende on 5/1/12.
//  Copyright (c) 2012 Pixel of View. All rights reserved.
//

#include "plugboard-lib-std.h"
#include "plugboard-utils.h"
#include <stdio.h>
#include <string.h>
#include "plugboard-lib-std-control.h"
#include "plugboard-lib-std-basictypes.h"
#include "plugboard-lib-std-basicarith.h"
#include "plugboard-lib-std-debug.h"


/*
 * the setup code proper
 */
void p_std_setup(void)
{
    p_std_control_setup();
    p_std_basictypes_setup();
    p_std_basicarith_setup();
    p_std_debug_setup();
    p_std_string_setup();
}

void p_default_destroy(void*unused) {}