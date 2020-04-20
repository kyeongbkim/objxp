#ifndef __C_INTF_H__
#define __C_INTF_H__

struct yobj_ctl {
  void (*free)();
};

#define yobj_alloc(type, args...)     yobj_alloc_ ## type (args)
#define yobj_putObject(type, args...) yobj_putObject_ ## type (args)
#define yobj_delObject(type, args...) yobj_delObject_ ## type (args)
extern void yobj_free(struct yobj_ctl* yctl);

#endif

