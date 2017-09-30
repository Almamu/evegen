#include "devtools.h"

#include <stdio.h>

static const char *devtools_script = R"(
import marshal
import os
import types

execfile('devtools.py')

k = 0
co_consts = list(Bootstrap.func_code.co_consts)

for i in Bootstrap.func_code.co_consts:
    ext = None
    fname = None
    if not isinstance(i, basestring):
        k = k + 1
        continue
    #print i.split('::')
    if i.split('::')[0] == 'hexex':
        ext = os.path.splitext(i)[1]
        fname = i.split('::')[1]
        print ' > Including %s' %(fname)
    else:
        k = k + 1
        continue
    #print ext
    if ext == '.py':
        f = open(fname, 'rb')
        c = f.read()
        code = compile(c, fname, 'exec')
        cc = marshal.dumps(code).encode('hex')
        co_consts[k] = cc
        f.close()
    k = k + 1

c = Bootstrap.func_code

BootstrapMod = types.CodeType(c.co_argcount,
                    # c.co_kwonlyargcount,  Add this in Python3
                    c.co_nlocals,
                    c.co_stacksize,
                    c.co_flags,
                    c.co_code,
                    tuple(co_consts),
                    c.co_names,
                    c.co_varnames,
                    c.co_filename,
                    c.co_name,
                    c.co_firstlineno,
                    c.co_lnotab,   # In general, You should adjust this
                    c.co_freevars,
                    c.co_cellvars)

print ' > Got %d co_consts ' %(len(c.co_consts))

#cobj = compile(Bootstrap, 'devtools', 'exec')
d = {'Bootstrap': (BootstrapMod, ('some', 'defaults'))}
f = open('devtools.raw', 'wb')
c = marshal.dump(d, f)
f.close()
)";

// WIP may never finish
bool MakeDevtoolsNoScript(const char *devtools_file) {
    PyObject *main = PyImport_AddModule("__main__");
    PyObject *globals = PyDict_New();
    PyObject *locals = PyDict_New();
    FILE *f = fopen(devtools_file);
    if (f == NULL) {
        printf(" ERROR | Couldn't find devtools file\n");
        return false;
    }

    PyRun_FileExFlags(f, devtools_file, Py_file_input,
                      globals, locals, true, NULL);

    int k = 0;

    PyObject *bootstrap = PyDict_GetItemString(locals, "Bootstrap");
    PyObject *func_code = PyObject_GetAttrString(bootstrap, "func_code");
    PyObject *co_consts_tuple = PyObject_GetAttrString(func_code, "co_consts");

    Py_ssize_t size = PyTuple_size(co_consts_tuple);
    PyObject *co_consts = PyList_New(size);

    return true;
}

bool MakeDevtools(const char *devtools_file) {
    int ret = PyRun_SimpleString(devtools_script);
    if (ret == 0) {
        printf(" OK | Wrote devtools.raw\n");
        return true;
    } else {
        printf(" ERROR | Couldn't write devtools.raw\n");
        return false;
    }
}
