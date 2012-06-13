//
// Copyright (C) 2012 Continuum Analytics
//
//
// The view dtype reinterprets the bytes of
// one dtype as another.
//
#ifndef _DND__VIEW_DTYPE_HPP_
#define _DND__VIEW_DTYPE_HPP_

#include <dnd/dtype.hpp>

namespace dnd {

class view_dtype : public extended_dtype {
    dtype m_value_dtype, m_operand_dtype;
public:
    view_dtype(const dtype& value_dtype, const dtype& operand_dtype)
        : m_value_dtype(value_dtype), m_operand_dtype(operand_dtype)
    {
        if (value_dtype.itemsize() != operand_dtype.value_dtype().itemsize()) {
            std::stringstream ss;
            ss << "view_dtype: Cannot view " << operand_dtype.value_dtype() << " as " << value_dtype << " because they have different sizes";
            throw std::runtime_error(ss.str());
        }
        if (value_dtype.is_object_type() || operand_dtype.is_object_type()) {
            throw std::runtime_error("view_dtype: Only POD dtypes are supported");
        }
    }

    type_id_t type_id() const {
        return view_type_id;
    }
    dtype_kind_t kind() const {
        return expression_kind;
    }
    // Expose the storage traits here
    unsigned char alignment() const {
        return m_operand_dtype.alignment();
    }
    uintptr_t itemsize() const {
        return m_operand_dtype.itemsize();
    }

    const dtype& value_dtype(const dtype& self) const {
        return m_value_dtype;
    }
    const dtype& operand_dtype(const dtype& self) const {
        return m_operand_dtype;
    }
    void print_data(std::ostream& o, const dtype& dt, const char *data, intptr_t stride, intptr_t size,
                        const char *separator) const;

    void print(std::ostream& o) const;

    // Don't support unaligned versions of object-semantic data
    bool is_object_type() const {
        return false;
    }

    bool is_lossless_assignment(const dtype& dst_dt, const dtype& src_dt) const;

    bool operator==(const extended_dtype& rhs) const;

    // For expression_kind dtypes - converts to/from the storage's value dtype
    void get_operand_to_value_operation(intptr_t dst_fixedstride, intptr_t src_fixedstride,
                        kernel_instance<unary_operation_t>& out_kernel) const;
    void get_value_to_operand_operation(intptr_t dst_fixedstride, intptr_t src_fixedstride,
                        kernel_instance<unary_operation_t>& out_kernel) const;
    dtype with_replaced_storage_dtype(const dtype& replacement_dtype) const;
};

/**
 * Makes an unaligned dtype to view the given dtype without alignment requirements.
 */
inline dtype make_view_dtype(const dtype& value_dtype, const dtype& operand_dtype) {
    if (value_dtype.kind() != expression_kind) {
        return dtype(make_shared<view_dtype>(value_dtype, operand_dtype));
    } else {
        // When the value dtype has an expression_kind, we need to chain things together
        // so that the view operation happens just at the primitive level.
        return value_dtype.extended()->with_replaced_storage_dtype(
            dtype(make_shared<view_dtype>(value_dtype.storage_dtype(), operand_dtype)));
    }
}

template<typename Tvalue, typename Toperand>
dtype make_view_dtype() {
    return dtype(make_shared<view_dtype>(make_dtype<Tvalue>()));
}

} // namespace dnd

#endif // _DND__VIEW_DTYPE_HPP_
