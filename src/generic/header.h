#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <libelf.h>
#include <gelf.h>
#include <fenv.h>
#include <limits.h>
#include <fstream>
#include "processor/constants.h"
#include <iostream>
#include "generic/utility.h"
#include <math.h>

#include<cstring>//Is this std?

#include "interface/eheader.h"

using namespace std;
typedef int regVal;
typedef unsigned int memAddress;

#endif
