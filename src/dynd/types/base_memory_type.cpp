//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/types/base_memory_type.hpp>
#include <dynd/func/make_callable.hpp>

using namespace std;
using namespace dynd;

base_memory_type::~base_memory_type()
{
}

size_t base_memory_type::get_default_data_size() const {
    if (m_storage_tp.is_builtin()) {
        return m_storage_tp.get_data_size();
    } else {
        return m_storage_tp.extended()->get_default_data_size();
    }
}

void base_memory_type::print_data(std::ostream &o, const char *arrmeta,
                                  const char *data) const
{
  m_storage_tp.print_data(o, arrmeta + m_storage_arrmeta_offset, data);
}

bool base_memory_type::is_lossless_assignment(const ndt::type& dst_tp, const ndt::type& src_tp) const
{
    // Default to calling with the storage types
    if (dst_tp.extended() == this) {
        return ::is_lossless_assignment(m_storage_tp, src_tp);
    } else {
        return ::is_lossless_assignment(dst_tp, m_storage_tp);
    }
}

void base_memory_type::transform_child_types(type_transform_fn_t transform_fn,
                                             intptr_t arrmeta_offset,
                                             void *extra,
                                             ndt::type &out_transformed_tp,
                                             bool &out_was_transformed) const
{
    ndt::type tmp_tp;
    bool was_transformed = false;
    transform_fn(m_storage_tp, arrmeta_offset + m_storage_arrmeta_offset, extra,
                 tmp_tp, was_transformed);
    if (was_transformed) {
        out_transformed_tp = with_replaced_storage_type(tmp_tp);
        out_was_transformed = true;
    } else {
        out_transformed_tp = ndt::type(this, true);
    }
}

ndt::type base_memory_type::get_canonical_type() const
{
    return m_storage_tp.get_canonical_type();
}

void base_memory_type::arrmeta_default_construct(char *arrmeta, bool blockref_alloc) const
{
  if (!m_storage_tp.is_builtin()) {
    m_storage_tp.extended()->arrmeta_default_construct(
        arrmeta + m_storage_arrmeta_offset, blockref_alloc);
  }
}

void base_memory_type::arrmeta_copy_construct(
    char *dst_arrmeta, const char *src_arrmeta,
    memory_block_data *embedded_reference) const
{
    if (!m_storage_tp.is_builtin()) {
        m_storage_tp.extended()->arrmeta_copy_construct(dst_arrmeta + m_storage_arrmeta_offset,
                        src_arrmeta + m_storage_arrmeta_offset, embedded_reference);
    }
}

void base_memory_type::arrmeta_destruct(char *arrmeta) const
{
    if (!m_storage_tp.is_builtin()) {
        m_storage_tp.extended()->arrmeta_destruct(arrmeta + m_storage_arrmeta_offset);
    }
}

static ndt::type property_get_storage_type(const ndt::type& tp) {
    const base_memory_type *md = tp.extended<base_memory_type>();
    return md->get_storage_type();
}

void base_memory_type::get_dynamic_type_properties(
                const std::pair<std::string, gfunc::callable> **out_properties,
                size_t *out_count) const
{
    static pair<string, gfunc::callable> type_properties[] = {
        pair<string, gfunc::callable>(
            "storage_type",
            gfunc::make_callable(&property_get_storage_type, "self"))};

    *out_properties = type_properties;
    *out_count = sizeof(type_properties) / sizeof(type_properties[0]);
}
