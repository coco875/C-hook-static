#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include <print-tree.h>
#include "tree-pass.h"
#include <tree-ssa-alias.h>
#include <gimple.h>
#include <gimple-expr.h>
#include <gimple-iterator.h>
#include <gimple-ssa.h>
#include <gimplify.h>
#include <stringpool.h>
#include <gimple-pretty-print.h>
#include <attribs.h>

/**
 * Name of the function called to profile code
 */
#define FUNCTION_NAME       "__inst_profile"

#ifdef _WIN32
__declspec(dllexport)
#endif
int plugin_is_GPL_compatible;

bool check_hook_args(tree args) {
    int type[] = {STRING_CST, FUNCTION_DECL, STRING_CST};
    int i = 0;
    for (tree arg = args; arg; arg = TREE_CHAIN(arg)) {
        if (TREE_CODE(TREE_VALUE(arg)) != type[i++]) {
            return false;
        }
    }
    return true;
}

/* Attribute handler callback */
static tree handle_hook_attribute(tree *node, tree name, tree args, int flags, bool *no_add_attrs) {
    if (TREE_CODE(*node) != FUNCTION_DECL) {
        perror("hook attribute can only be applied to functions");
        return NULL_TREE;
    }
    // debug_tree(args);
    if (!check_hook_args(args)) {
        perror("hook attribute arguments are invalid");
        return NULL_TREE;
    }
    return NULL_TREE;
}

/* Attribute handler callback */
static tree handle_print_func_attribute(tree *node, tree name, tree args, int flags, bool *no_add_attrs) {
    if (TREE_CODE(*node) != FUNCTION_DECL) {
        perror("print_func attribute can only be applied to functions");
        return NULL_TREE;
    }
    debug_tree(*node);
    return NULL_TREE;
}

/* Attribute definition */
static struct attribute_spec hook_attr =
    { "hook", 3, 5, false,  false, false, false, handle_hook_attribute, NULL };

/* Attribute definition */
static struct attribute_spec print_func_attr =
    { "print_func", 0, 0, false,  false, false, false, handle_print_func_attribute, NULL };

/* Plugin callback called during attribute registration.
Registered with register_callback (plugin_name, PLUGIN_ATTRIBUTES, register_attributes, NULL)
*/
static void register_attributes(void *event_data, void *data) {
    register_attribute(&hook_attr);
    register_attribute(&print_func_attr);
}

// -----------------------------------------------------------------------------
// PLUGIN INSTRUMENTATION LOGICS
// -----------------------------------------------------------------------------

/**
 * Create a function call to '__profile' and insert it before the given stmt
 */
static void insert_instrumentation_fn(gimple* curr_stmt)
{
    // build function prototype
    tree proto = build_function_type_list(
            void_type_node,             // return type
            NULL_TREE                   // varargs terminator
        );           
    
    // builds and returns function declaration with NAME and PROTOTYPE
    tree decl = build_fn_decl(FUNCTION_NAME, proto);

    // build the GIMPLE function call to decl
    gcall* call = gimple_build_call(decl, 0);

    // get an iterator pointing to first basic block of the statement
    gimple_stmt_iterator gsi = gsi_for_stmt(curr_stmt);

    // insert it before the statement that was passed as the first argument
    gsi_insert_before(&gsi, call, GSI_NEW_STMT);
}

/**
 * For each function lookup attributes and attach profiling function
 */
static unsigned int instrument_assignments_plugin_exec(void)
{
    // get the FUNCTION_DECL of the function whose body we are reading
    tree fndef = current_function_decl;
    
    // print the function name
    fprintf(stderr, "> Inspecting function '%s'\n", IDENTIFIER_POINTER (DECL_NAME (fndef)));

    // get the attributes list
    tree attrlist = DECL_ATTRIBUTES(fndef);

    // lookup into attribute list searcing for our registered attribute
    tree attr = lookup_attribute("print_func", attrlist);

    // if the attribute is not present
    if (attr == NULL_TREE)
        return 0;

    // attribute was in the list
    fprintf(stderr, "\t attribute %s found! \n", "print_func");

    // get function entry block
    basic_block entry = ENTRY_BLOCK_PTR_FOR_FN(cfun)->next_bb;

    for (basic_block bb = entry; bb; bb = bb->next_bb) {
        debug_bb(bb);
    }

    // get the first statement
    gimple* first_stmt = gsi_stmt(gsi_start_bb(entry));

    // warn the user we are adding a profiling function
    fprintf(stderr, "\t adding function call before ");
    print_gimple_stmt(stderr, first_stmt, 0, TDF_NONE);

    // insert the function
    insert_instrumentation_fn(first_stmt);

    // done!
    return 0;
}

/** 
 * Metadata for a pass, non-varying across all instances of a pass
 * @see Declared in tree-pass.h
 * @note Refer to tree-pass for docs about
 */
struct pass_data ins_pass_data =
{
    .type = GIMPLE_PASS,                                    // type of pass
    .name = "hook",                                    // name of plugin
    .optinfo_flags = OPTGROUP_NONE,                         // no opt dump
    .tv_id = TV_NONE,                                       // no timevar (see timevar.h)
    .properties_required = PROP_gimple_any,                 // entire gimple grammar as input
    .properties_provided = 0,                               // no prop in output
    .properties_destroyed = 0,                              // no prop removed
    .todo_flags_start = 0,                                  // need nothing before
    .todo_flags_finish = TODO_update_ssa|TODO_cleanup_cfg   // need to update SSA repr after and repair cfg
};

/**
 * Definition of our instrumentation GIMPLE pass
 * @note Extends gimple_opt_pass class
 * @see Declared in tree-pass.h
 */
class ins_gimple_pass : public gimple_opt_pass
{
public:

    /**
     * Constructor
     */
    ins_gimple_pass (const pass_data& data, gcc::context *ctxt) : gimple_opt_pass (data, ctxt) {}

    /**
     * This and all sub-passes are executed only if the function returns true
     * @note Defined in opt_pass father class
     * @see Defined in tree-pass.h
     */ 
    bool gate (function* gate_fun) 
    {
        return true;
    }

    /**
     * This is the code to run when pass is executed
     * @note Defined in opt_pass father class
     * @see Defined in tree-pass.h
     */
    unsigned int execute(function* exec_fun)
    {
        return instrument_assignments_plugin_exec();
    }
};

// instanciate a new instrumentation GIMPLE pass
ins_gimple_pass inst_pass = ins_gimple_pass(ins_pass_data, NULL);

#ifdef _WIN32
__declspec(dllexport)
#endif
int plugin_init (struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    const char *plugin_name = plugin_info->base_name;
    struct register_pass_info pass;

    // insert inst pass into the struct used to register the pass
    pass.pass = &inst_pass;

    // and get called after GCC has produced SSA representation  
    pass.reference_pass_name = "ssa";

    // after the first opt pass to be sure opt will not throw away our stuff
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    // add our pass hooking into pass manager
    register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);

    register_callback(plugin_name, PLUGIN_ATTRIBUTES, register_attributes, NULL);
    return 0;
}
