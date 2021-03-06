//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/type.hpp>
#include <dynd/types/static_type_instances.hpp>

namespace dynd {

/**
 * A dynd type whose nd::array instances contain
 * arrfunc objects.
 */
class arrfunc_old_type : public base_type {
public:
    arrfunc_old_type();

    virtual ~arrfunc_old_type();

    void print_data(std::ostream& o, const char *arrmeta, const char *data) const;

    void print_type(std::ostream& o) const;

    bool operator==(const base_type& rhs) const;

    void arrmeta_default_construct(char *arrmeta, bool blockref_alloc) const;
    void arrmeta_copy_construct(char *dst_arrmeta, const char *src_arrmeta, memory_block_data *embedded_reference) const;
    void arrmeta_reset_buffers(char *arrmeta) const;
    void arrmeta_finalize_buffers(char *arrmeta) const;
    void arrmeta_destruct(char *arrmeta) const;

    void data_destruct(const char *arrmeta, char *data) const;
    void data_destruct_strided(const char *arrmeta, char *data,
                    intptr_t stride, size_t count) const;

    size_t make_assignment_kernel(void *ckb, intptr_t ckb_offset,
                                  const ndt::type &dst_tp,
                                  const char *dst_arrmeta,
                                  const ndt::type &src_tp,
                                  const char *src_arrmeta,
                                  kernel_request_t kernreq,
                                  const eval::eval_context *ectx) const;
};

namespace ndt {
  inline const ndt::type &make_arrfunc()
  {
    return *reinterpret_cast<const ndt::type *>(&types::arrfunc_tp);
  }
} // namespace ndt

} // namespace dynd
