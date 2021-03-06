//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/type.hpp>
#include <dynd/types/base_dim_type.hpp>
#include <dynd/typed_data_assign.hpp>
#include <dynd/types/view_type.hpp>
#include <dynd/types/string_type.hpp>
#include <dynd/types/type_type.hpp>
#include <dynd/array.hpp>

namespace dynd {

class fixed_dimsym_type : public base_dim_type {
public:
  fixed_dimsym_type(const ndt::type &element_tp);

  virtual ~fixed_dimsym_type();

  size_t get_default_data_size() const;

  void print_data(std::ostream &o, const char *arrmeta, const char *data) const;

  void print_type(std::ostream &o) const;

  bool is_expression() const;
  bool is_unique_data_owner(const char *arrmeta) const;
  void transform_child_types(type_transform_fn_t transform_fn,
                             intptr_t arrmeta_offset, void *extra,
                             ndt::type &out_transformed_tp,
                             bool &out_was_transformed) const;
  ndt::type get_canonical_type() const;


  ndt::type at_single(intptr_t i0, const char **inout_arrmeta,
                      const char **inout_data) const;

  ndt::type get_type_at_dimension(char **inout_arrmeta, intptr_t i,
                                  intptr_t total_ndim = 0) const;

  intptr_t get_dim_size(const char *arrmeta, const char *data) const;
  void get_shape(intptr_t ndim, intptr_t i, intptr_t *out_shape,
                 const char *arrmeta, const char *data) const;

  bool is_lossless_assignment(const ndt::type &dst_tp,
                              const ndt::type &src_tp) const;

  bool operator==(const base_type &rhs) const;

  void arrmeta_default_construct(char *arrmeta, bool blockref_alloc) const;
  void arrmeta_copy_construct(char *dst_arrmeta, const char *src_arrmeta,
                              memory_block_data *embedded_reference) const;
  void arrmeta_reset_buffers(char *arrmeta) const;
  void arrmeta_finalize_buffers(char *arrmeta) const;
  void arrmeta_destruct(char *arrmeta) const;
  void arrmeta_debug_print(const char *arrmeta, std::ostream &o,
                           const std::string &indent) const;
  size_t
  arrmeta_copy_construct_onedim(char *dst_arrmeta, const char *src_arrmeta,
                                memory_block_data *embedded_reference) const;

  void data_destruct(const char *arrmeta, char *data) const;
  void data_destruct_strided(const char *arrmeta, char *data, intptr_t stride,
                             size_t count) const;

  void get_dynamic_type_properties(
      const std::pair<std::string, gfunc::callable> **out_properties,
      size_t *out_count) const;
  void get_dynamic_array_properties(
      const std::pair<std::string, gfunc::callable> **out_properties,
      size_t *out_count) const;
  void get_dynamic_array_functions(
      const std::pair<std::string, gfunc::callable> **out_functions,
      size_t *out_count) const;
};

namespace ndt {
  ndt::type make_fixed_dimsym(const ndt::type &element_tp);

  inline ndt::type make_fixed_dimsym(const ndt::type &uniform_tp,
                                      intptr_t ndim)
  {
    if (ndim > 0) {
      ndt::type result = make_fixed_dimsym(uniform_tp);
      for (intptr_t i = 1; i < ndim; ++i) {
        result = make_fixed_dimsym(result);
      }
      return result;
    }
    else {
      return uniform_tp;
    }
  }
} // namespace ndt

} // namespace dynd
