//
// Copyright (C) 2011-13, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/kernels/expr_kernels.hpp>

using namespace std;
using namespace dynd;


namespace {
template<int N>
struct expression_dtype_expr_kernel_extra {
    typedef expression_dtype_expr_kernel_extra extra_type;

    bool is_expr[N];

    
};

} // anonymous namespace

size_t dynd::make_expression_dtype_expr_kernel(hierarchical_kernel *DYND_UNUSED(out), size_t DYND_UNUSED(offset_out),
                const dtype& DYND_UNUSED(dst_dt), const char *DYND_UNUSED(dst_metadata),
                size_t DYND_UNUSED(src_count), const dtype *DYND_UNUSED(src_dt), const char **DYND_UNUSED(src_metadata),
                kernel_request_t DYND_UNUSED(kernreq), const eval::eval_context *DYND_UNUSED(ectx),
                const expr_kernel_generator *DYND_UNUSED(handler))
{
    throw runtime_error("TODO: make_expression_dtype_expr_kernel");
}