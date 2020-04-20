#include "c-intf.h"

void yobj_free(struct yobj_ctl* yctl) {
  yctl->free(yctl);
}

