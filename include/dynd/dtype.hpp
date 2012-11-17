//
// Copyright (C) 2011-12, Dynamic NDArray Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef _DYND__DTYPE_HPP_
#define _DYND__DTYPE_HPP_

#include <iostream>
#include <complex>
#include <stdexcept>
#include <vector>

#include <boost/detail/atomic_count.hpp>

#include <dynd/config.hpp>
#include <dynd/dtype_assign.hpp>
#include <dynd/dtype_comparisons.hpp>
#include <dynd/kernels/single_compare_kernel_instance.hpp>
#include <dynd/kernels/unary_kernel_instance.hpp>
#include <dynd/string_encodings.hpp>
#include <dynd/eval/eval_context.hpp>
#include <dynd/irange.hpp>

namespace dynd {

// A boolean class for dynamicndarray which is one-byte big
class dynd_bool {
    char m_value;
public:
    dynd_bool() : m_value(0) {}

    dynd_bool(bool value) : m_value(value) {}

    // Special case complex conversion to avoid ambiguous overload
    template<class T>
    dynd_bool(std::complex<T> value) : m_value(value != std::complex<T>(0)) {}

    operator bool() const {
        return m_value != 0;
    }
};

enum dtype_kind_t {
    bool_kind,
    int_kind,
    uint_kind,
    real_kind,
    complex_kind,
    // string_kind means subclass of extended_string_dtype
    string_kind,
    bytes_kind,
    void_kind,
    datetime_kind,
    // For struct_type_id and ndarray_type_id
    composite_kind,
    // For dtypes whose value_dtype != the dtype, signals
    // that calculations should look at the value_dtype for
    // type promotion, etc.
    expression_kind,
    // For pattern-matching dtypes
    pattern_kind,
    // For use when it becomes possible to register custom dtypes
    custom_kind
};

enum type_id_t {
    // A 1-byte boolean type
    bool_type_id,
    // Signed integer types
    int8_type_id,
    int16_type_id,
    int32_type_id,
    int64_type_id,
    // Unsigned integer types
    uint8_type_id,
    uint16_type_id,
    uint32_type_id,
    uint64_type_id,
    // Floating point types
    float32_type_id,
    float64_type_id,
    // Complex floating-point types
    complex_float32_type_id,
    complex_float64_type_id,
    // Means no type, just like in C. (Different from Numpy)
    void_type_id,
    void_pointer_type_id,

    // Other primitives (not builtin)
    fixedbytes_type_id,
    fixedstring_type_id,
    categorical_type_id,
    date_type_id,
    busdate_type_id,
    pointer_type_id,

    // blockref primitive dtypes
    string_type_id,

    // blockref composite dtypes
    array_type_id,

    // Composite dtypes
    strided_array_type_id,
    struct_type_id,
    tuple_type_id,
    ndarray_type_id,

    // Adapter dtypes
    convert_type_id,
    byteswap_type_id,
    align_type_id,
    view_type_id,

    // pattern matches against other types - cannot instantiate
    pattern_type_id,

    // The number of built-in, atomic types
    builtin_type_id_count = 13
};

enum {
    /** A mask within which alll the built-in type ids are guaranteed to fit */
    builtin_type_id_mask = 0x1f
};

enum dtype_memory_management_t {
    /** The dtype's memory is POD (plain old data) */
    pod_memory_management,
    /** The dtype contains pointers into another memory_block */
    blockref_memory_management,
    /** The dtype requires full object lifetime management (construct/copy/move/destroy) */
    object_memory_management
};


namespace detail {
    // Simple metaprogram taking log base 2 of 1, 2, 4, and 8
    template <int I> struct log2_x;
    template <> struct log2_x<1> {
        enum {value = 0};
    };
    template <> struct log2_x<2> {
        enum {value = 1};
    };
    template <> struct log2_x<4> {
        enum {value = 2};
    };
    template <> struct log2_x<8> {
        enum {value = 3};
    };
}


// Type trait for the type id
template <typename T> struct type_id_of;

// Can't use bool, because it doesn't have a guaranteed sizeof
template <> struct type_id_of<dynd_bool> {enum {value = bool_type_id};};
template <> struct type_id_of<char> {enum {value = ((char)-1) < 0 ? int8_type_id : uint8_type_id};};
template <> struct type_id_of<signed char> {enum {value = int8_type_id};};
template <> struct type_id_of<short> {enum {value = int16_type_id};};
template <> struct type_id_of<int> {enum {value = int32_type_id};};
template <> struct type_id_of<long> {
    enum {value = int8_type_id + detail::log2_x<sizeof(long)>::value};
};
template <> struct type_id_of<long long> {enum {value = int64_type_id};};
template <> struct type_id_of<uint8_t> {enum {value = uint8_type_id};};
template <> struct type_id_of<uint16_t> {enum {value = uint16_type_id};};
template <> struct type_id_of<unsigned int> {enum {value = uint32_type_id};};
template <> struct type_id_of<unsigned long> {
    enum {value = uint8_type_id + detail::log2_x<sizeof(unsigned long)>::value};
};
template <> struct type_id_of<unsigned long long>{enum {value = uint64_type_id};};
template <> struct type_id_of<float> {enum {value = float32_type_id};};
template <> struct type_id_of<double> {enum {value = float64_type_id};};
template <> struct type_id_of<std::complex<float> > {enum {value = complex_float32_type_id};};
template <> struct type_id_of<std::complex<double> > {enum {value = complex_float64_type_id};};
template <> struct type_id_of<void> {enum {value = void_type_id};};

// Type trait for the kind
template <typename T> struct dtype_kind_of;

template <> struct dtype_kind_of<void> {static const dtype_kind_t value = void_kind;};
// Can't use bool, because it doesn't have a guaranteed sizeof
template <> struct dtype_kind_of<dynd_bool> {static const dtype_kind_t value = bool_kind;};
template <> struct dtype_kind_of<char> {
    static const dtype_kind_t value = ((char)-1) < 0 ? int_kind : uint_kind;
};
template <> struct dtype_kind_of<signed char> {static const dtype_kind_t value = int_kind;};
template <> struct dtype_kind_of<short> {static const dtype_kind_t value = int_kind;};
template <> struct dtype_kind_of<int> {static const dtype_kind_t value = int_kind;};
template <> struct dtype_kind_of<long> {static const dtype_kind_t value = int_kind;};
template <> struct dtype_kind_of<long long> {static const dtype_kind_t value = int_kind;};
template <> struct dtype_kind_of<uint8_t> {static const dtype_kind_t value = uint_kind;};
template <> struct dtype_kind_of<uint16_t> {static const dtype_kind_t value = uint_kind;};
template <> struct dtype_kind_of<unsigned int> {static const dtype_kind_t value = uint_kind;};
template <> struct dtype_kind_of<unsigned long> {static const dtype_kind_t value = uint_kind;};
template <> struct dtype_kind_of<unsigned long long>{static const dtype_kind_t value = uint_kind;};
template <> struct dtype_kind_of<float> {static const dtype_kind_t value = real_kind;};
template <> struct dtype_kind_of<double> {static const dtype_kind_t value = real_kind;};
template <typename T> struct dtype_kind_of<std::complex<T> > {static const dtype_kind_t value = complex_kind;};

// Metaprogram for determining if a type is a valid C++ scalar
// of a particular dtype.
template<typename T> struct is_dtype_scalar {enum {value = false};};
template <> struct is_dtype_scalar<dynd_bool> {enum {value = true};};
template <> struct is_dtype_scalar<char> {enum {value = true};};
template <> struct is_dtype_scalar<signed char> {enum {value = true};};
template <> struct is_dtype_scalar<short> {enum {value = true};};
template <> struct is_dtype_scalar<int> {enum {value = true};};
template <> struct is_dtype_scalar<long> {enum {value = true};};
template <> struct is_dtype_scalar<long long> {enum {value = true};};
template <> struct is_dtype_scalar<unsigned char> {enum {value = true};};
template <> struct is_dtype_scalar<unsigned short> {enum {value = true};};
template <> struct is_dtype_scalar<unsigned int> {enum {value = true};};
template <> struct is_dtype_scalar<unsigned long> {enum {value = true};};
template <> struct is_dtype_scalar<unsigned long long> {enum {value = true};};
template <> struct is_dtype_scalar<float> {enum {value = true};};
template <> struct is_dtype_scalar<double> {enum {value = true};};
template <> struct is_dtype_scalar<std::complex<float> > {enum {value = true};};
template <> struct is_dtype_scalar<std::complex<double> > {enum {value = true};};

// Metaprogram for determining scalar alignment
template<typename T> struct scalar_align_of;
template <> struct scalar_align_of<dynd_bool> {enum {value = 1};};
template <> struct scalar_align_of<char> {enum {value = 1};};
template <> struct scalar_align_of<signed char> {enum {value = 1};};
template <> struct scalar_align_of<short> {enum {value = sizeof(short)};};
template <> struct scalar_align_of<int> {enum {value = sizeof(int)};};
template <> struct scalar_align_of<long> {enum {value = sizeof(long)};};
template <> struct scalar_align_of<long long> {enum {value = sizeof(long long)};};
template <> struct scalar_align_of<unsigned char> {enum {value = 1};};
template <> struct scalar_align_of<unsigned short> {enum {value = sizeof(unsigned short)};};
template <> struct scalar_align_of<unsigned int> {enum {value = sizeof(unsigned int)};};
template <> struct scalar_align_of<unsigned long> {enum {value = sizeof(unsigned long)};};
template <> struct scalar_align_of<unsigned long long> {enum {value = sizeof(unsigned long long)};};
template <> struct scalar_align_of<float> {enum {value = sizeof(long)};};
template <> struct scalar_align_of<double> {enum {value = sizeof(double)};};
template <> struct scalar_align_of<std::complex<float> > {enum {value = sizeof(long)};};
template <> struct scalar_align_of<std::complex<double> > {enum {value = sizeof(double)};};

// Metaprogram for determining if a type is the C++ "bool" or not
template<typename T> struct is_type_bool {enum {value = false};};
template<> struct is_type_bool<bool> {enum {value = true};};

/**
 * Increments the offset value so that it is aligned to the requested alignment
 * NOTE: The alignment must be a power of two.
 */
inline size_t inc_to_alignment(size_t offset, size_t alignment) {
    return (offset + alignment - 1) & (size_t)(-(ptrdiff_t)alignment);
}

/**
 * Increments the pointer value so that it is aligned to the requested alignment
 * NOTE: The alignment must be a power of two.
 */
inline char *inc_to_alignment(char *ptr, size_t alignment) {
    return reinterpret_cast<char *>((reinterpret_cast<size_t>(ptr) + alignment - 1) & (size_t)(-(ptrdiff_t)alignment));
}

/**
 * Increments the pointer value so that it is aligned to the requested alignment
 * NOTE: The alignment must be a power of two.
 */
inline void *inc_to_alignment(void *ptr, size_t alignment) {
    return reinterpret_cast<char *>((reinterpret_cast<size_t>(ptr) + alignment - 1) & (size_t)(-(ptrdiff_t)alignment));
}

/** Prints a single scalar of a builtin dtype to the stream */
void print_builtin_scalar(type_id_t type_id, std::ostream& o, const char *data);

class dtype;
class extended_dtype;
struct iterdata_common;

/** This is the callback function type used by the extended_dtype::foreach function */
typedef void (*foreach_fn_t)(const extended_dtype *dt, char *data, const char *metadata, const void *callback_data);

/**
 * This is the iteration increment function used by iterdata. It increments the
 * iterator at the specified level, resetting all the more inner levels to 0.
 */
typedef char * (*iterdata_increment_fn_t)(iterdata_common *iterdata, int level);
/**
 * This is the reset function which is called when an outer dimension
 * increment resets all the lower dimensions to index 0. It returns
 * the data pointer for the next inner level of iteration.
 */
typedef char * (*iterdata_reset_fn_t)(iterdata_common *iterdata, char *data, int ndim);

struct iterdata_common {
    iterdata_increment_fn_t incr;
    iterdata_reset_fn_t reset;
};

// The extended_dtype class is for dtypes which require more data
// than a type_id, kind, and element_size, and endianness.
class extended_dtype {
    /** Embedded reference counting */
    mutable boost::detail::atomic_count m_use_count;
public:
    /** Starts off the extended dtype instance with a use count of 1. */
    extended_dtype()
        : m_use_count(1)
    {}

    virtual ~extended_dtype();

    virtual type_id_t type_id() const = 0;
    virtual dtype_kind_t kind() const = 0;
    virtual size_t alignment() const = 0;
    virtual size_t get_element_size() const = 0;
    virtual size_t get_default_element_size(int ndim, const intptr_t *shape) const;

    /**
     * Print the raw data interpreted as a single value of this dtype.
     *
     * @param o the std::ostream to print to
     * @param data pointer to the data element to print
     */
    virtual void print_element(std::ostream& o, const char *data, const char *metadata) const = 0;

    /**
     * Print a representation of the dtype itself
     *
     * @param o the std::ostream to print to
     */
    virtual void print_dtype(std::ostream& o) const = 0;

    /** Returns what kind of memory management the dtype uses, e.g. construct/copy/move/destruct semantics */
    virtual dtype_memory_management_t get_memory_management() const = 0;

    /** Returns true if the ndobject with the data/metadata is a scalar */
    virtual bool is_scalar(const char *data, const char *metadata) const;

    /**
     * Indexes into the dtype. This function returns the dtype which results
     * from applying the same index to an ndarray of this dtype.
     *
     * @param nindices     The number of elements in the 'indices' array. This is shrunk by one for each recursive call.
     * @param indices      The indices to apply. This is incremented by one for each recursive call.
     * @param current_i    The current index position. Used for error messages.
     * @param root_dt      The data type in the first call, before any recursion. Used for error messages.
     */
    virtual dtype apply_linear_index(int nindices, const irange *indices, int current_i, const dtype& root_dt) const;

    /**
     * Retrieves the shape of the dtype, expanding the vector as needed. For dimensions with
     * unknown or variable shape, -1 is returned.
     */
    virtual void get_shape(int i, std::vector<intptr_t>& out_shape) const;

    /**
     * Retrieves the shape of the dtype ndobject instance, expanding the vector as needed. For dimensions with
     * variable shape, -1 is returned.
     */
    virtual void get_shape(int i, std::vector<intptr_t>& out_shape, const char *data, const char *metadata) const;

    /**
     * Retrieves the strides of the dtype ndobject instance, expanding the vector as needed. For dimensions
     * where there is not a simple stride (e.g. a tuple/struct dtype), 0 is returned and
     * the caller should handle this.
     */
    virtual void get_strides(int i, std::vector<intptr_t>& out_strides, const char *data, const char *metadata) const;

    /**
     * Called by ::dynd::is_lossless_assignment, with (this == dst_dt->extended()).
     */
    virtual bool is_lossless_assignment(const dtype& dst_dt, const dtype& src_dt) const = 0;

    /*
     * Return a comparison kernel that can perform the requested single comparison on
     * data of this dtype
     *
     * @param compare_id the identifier of the comparison
     */
    virtual void get_single_compare_kernel(single_compare_kernel_instance& out_kernel) const;

    /**
     * Called by ::dynd::get_dtype_assignment_kernel with (this == dst_dt.extended()) or
     * by another implementation of this function with (this == src_dt.extended()).
     *
     * If (this == dst_dt.extended()), and the function can't produce an assignment kernel,
     * should call dst_dt.extended()->get_dtype_assignment_kernel(...) to let the other
     * dtype provide the function if it can be done.
     */
    virtual void get_dtype_assignment_kernel(const dtype& dst_dt, const dtype& src_dt,
                    assign_error_mode errmode,
                    unary_specialization_kernel_instance& out_kernel) const;

    virtual bool operator==(const extended_dtype& rhs) const = 0;

    /** The size of the ndobject metadata for this dtype */
    virtual size_t get_metadata_size() const;
    /**
     * Constructs the ndobject metadata for this dtype, prepared for writing.
     * The element size of the result must match that from get_default_element_size().
     */
    virtual void metadata_default_construct(char *metadata, int ndim, const intptr_t* shape) const;
    /** Destructs any references or other state contained in the ndobjects' metdata */
    virtual void metadata_destruct(char *metadata) const;
    /** Debug print of the metdata */
    virtual void metadata_debug_dump(const char *metadata, std::ostream& o, const std::string& indent) const;

    /** The size of the data required for uniform iteration */
    virtual size_t get_iterdata_size() const;
    /**
     * Constructs the iterdata for processing iteration at this level of the datashape
     */
    virtual size_t iterdata_construct(iterdata_common *iterdata, const char *metadata, int ndim, const intptr_t* shape, dtype& out_uniform_dtype) const;
    /** Destructs any references or other state contained in the iterdata */
    virtual size_t iterdata_destruct(iterdata_common *iterdata, int ndim) const;

    /**
     * Call the callback on each element of the array with given data/metdata, descending
     * to exactly 'ndim' dimensions (or all the way if ndim is -1).
     */
    virtual void foreach(int ndim, char *data, const char *metadata, foreach_fn_t callback, const void *callback_data) const;

    friend void extended_dtype_incref(const extended_dtype *ed);
    friend void extended_dtype_decref(const extended_dtype *ed);
};

/**
 * Increments the reference count of a memory block object.
 */
inline void extended_dtype_incref(const extended_dtype *ed)
{
    ++ed->m_use_count;
}

/**
 * Decrements the reference count of a memory block object,
 * freeing it if the count reaches zero.
 */
inline void extended_dtype_decref(const extended_dtype *ed)
{
    if (--ed->m_use_count == 0) {
        delete ed;
    }
}



/**
 * Base class for all string extended dtypes. If a dtype
 * has kind string_kind, it must be a subclass of
 * extended_string_dtype.
 */
class extended_string_dtype : public extended_dtype {
public:
    virtual ~extended_string_dtype();
    /** The encoding used by the string */
    virtual string_encoding_t encoding() const = 0;
};

/**
 * Base class for all dtypes of expression_kind.
 */
class extended_expression_dtype : public extended_dtype {
public:
    /**
     * Should return a reference to the dtype representing the value which
     * is for calculation. This should never be an expression dtype.
     */
    virtual const dtype& get_value_dtype() const = 0;
    /**
     * Should return a reference to a dtype representing the data this dtype
     * uses to produce the value.
     */
    virtual const dtype& get_operand_dtype() const = 0;

    /** Returns a kernel which converts from (operand_dtype().value_dtype()) to (value_dtype()) */
    virtual void get_operand_to_value_kernel(const eval::eval_context *ectx,
                            unary_specialization_kernel_instance& out_borrowed_kernel) const = 0;
    /** Returns a kernel which converts from (value_dtype()) to (operand_dtype().value_dtype()) */
    virtual void get_value_to_operand_kernel(const eval::eval_context *ectx,
                            unary_specialization_kernel_instance& out_borrowed_kernel) const = 0;

    /**
     * This method is for expression dtypes, and is a way to substitute
     * the storage dtype (deepest operand dtype) of an existing dtype.
     *
     * The value_dtype of the replacement should match the storage dtype
     * of this instance. Implementations of this should raise an exception
     * when this is not true.
     */
    virtual dtype with_replaced_storage_dtype(const dtype& replacement_dtype) const = 0;
};

namespace detail {
    /**
     * Internal implementation detail - makes a builtin dtype from its raw values.
     */
    /* TODO: DYND_CONSTEXPR */ dtype internal_make_raw_dtype(char type_id, char kind, intptr_t element_size, char alignment);

} // namespace detail


/**
 * This class represents a data type.
 *
 * The purpose of this data type is to describe the data layout
 * of elements in ndarrays. The class stores a number of common
 * properties, like a type id, a kind, an alignment, a byte-swapped
 * flag, and an element_size. Some data types have additional data
 * which is stored as a dynamically allocated extended_dtype object.
 *
 * For the simple built-in dtypes, no extended data is needed, in
 * which case this is entirely a value type with no allocated memory.
 *
 */
class dtype {
private:
    unsigned char m_type_id, m_kind, m_alignment;
    size_t m_element_size;
    const extended_dtype *m_extended;

    /** Unchecked built-in dtype constructor from raw parameters */
    /* TODO: DYND_CONSTEXPR */ dtype(char type_id, char kind, size_t element_size, char alignment)
        : m_type_id(type_id), m_kind(kind),
          m_alignment(alignment), m_element_size(element_size), m_extended(NULL)
    {}
public:
    /** Constructor */
    dtype();
    /** Constructor from an extended_dtype. This claims ownership of the 'extended' reference by default, be careful! */
    explicit dtype(const extended_dtype *extended, bool incref = false)
        : m_type_id(extended->type_id()), m_kind(extended->kind()), m_alignment((unsigned char)extended->alignment()),
            m_element_size(extended->get_element_size()), m_extended(extended) {
        if (incref) {
            extended_dtype_incref(m_extended);
        }
    }
    /** Copy constructor (should be "= default" in C++11) */
    dtype(const dtype& rhs)
        : m_type_id(rhs.m_type_id), m_kind(rhs.m_kind), m_alignment(rhs.m_alignment),
          m_element_size(rhs.m_element_size), m_extended(rhs.m_extended)
    {
        if (m_extended != NULL) {
            extended_dtype_incref(m_extended);
        }
    }
    /** Assignment operator (should be "= default" in C++11) */
    dtype& operator=(const dtype& rhs) {
        m_type_id = rhs.m_type_id;
        m_kind = rhs.m_kind;
        m_alignment = rhs.m_alignment;
        m_element_size = rhs.m_element_size;
        m_extended = rhs.m_extended;
        if (m_extended != NULL) {
            extended_dtype_incref(m_extended);
        }
        return *this;
    }
#ifdef DYND_RVALUE_REFS
    /** Move constructor (should be "= default" in C++11) */
    dtype(dtype&& rhs)
        : m_type_id(rhs.m_type_id), m_kind(rhs.m_kind), m_alignment(rhs.m_alignment),
          m_element_size(rhs.m_element_size),
          m_extended(rhs.m_extended)
    {
        rhs.m_extended = NULL;
    }
    /** Move assignment operator (should be "= default" in C++11) */
    dtype& operator=(dtype&& rhs) {
        m_type_id = rhs.m_type_id;
        m_kind = rhs.m_kind;
        m_alignment = rhs.m_alignment;
        m_element_size = rhs.m_element_size;
        m_extended = rhs.m_extended;
        rhs.m_extended = NULL;
        return *this;
    }
#endif // DYND_RVALUE_REFS

    /** Construct from a type ID */
    explicit dtype(type_id_t type_id);
    explicit dtype(int type_id);

    /** Construct from a string representation */
    explicit dtype(const std::string& rep);

    ~dtype() {
        if (m_extended != NULL) {
            extended_dtype_decref(m_extended);
        }
    }

    void swap(dtype& rhs) {
        std::swap(m_type_id, rhs.m_type_id);
        std::swap(m_kind, rhs.m_kind);
        std::swap(m_alignment, rhs.m_alignment);
        std::swap(m_element_size, rhs.m_element_size);
        std::swap(m_extended, rhs.m_extended);
    }

    bool operator==(const dtype& rhs) const {
        if (m_extended && rhs.m_extended) {
            return *m_extended == *rhs.m_extended;
        }
        return m_type_id == rhs.m_type_id &&
                m_element_size == rhs.m_element_size &&
                m_kind == rhs.m_kind &&
                m_alignment == rhs.m_alignment &&
                m_extended == rhs.m_extended;
    }
    bool operator!=(const dtype& rhs) const {
        return !(operator==(rhs));
    }

    /**
     * Indexes into the dtype. This function returns the dtype which results
     * from applying the same index to an ndarray of this dtype.
     *
     * @param ndim         The number of elements in the 'indices' array
     * @param indices      The indices to apply.
     */
    dtype index(int nindices, const irange *indices) const;

    /**
     * Indexes into the dtype, intended for recursive calls from the extended-dtype version. See
     * the function in extended_dtype with the same name for more details.
     */
    dtype apply_linear_index(int nindices, const irange *indices, int current_i, const dtype& root_dt) const;

    /**
     * Returns the non-expression dtype that this
     * dtype looks like for the purposes of calculation,
     * printing, etc.
     */
    const dtype& value_dtype() const {
        // Only expression_kind dtypes have different value_dtype
        if (m_kind != expression_kind) {
            return *this;
        } else {
            // All chaining happens in the operand_dtype
            return static_cast<const extended_expression_dtype *>(m_extended)->get_value_dtype();
        }
    }

    /**
     * For expression dtypes, returns the operand dtype,
     * which is the source dtype of this dtype's expression.
     * This is one link down the expression chain.
     */
    const dtype& operand_dtype() const {
        // Only expression_kind dtypes have different operand_dtype
        if (m_kind != expression_kind) {
            return *this;
        } else {
            return static_cast<const extended_expression_dtype *>(m_extended)->get_operand_dtype();
        }
    }

    /**
     * For expression dtypes, returns the storage dtype,
     * which is the dtype of the underlying input data.
     * This is the bottom of the expression chain.
     */
    const dtype& storage_dtype() const {
        // Only expression_kind dtypes have different storage_dtype
        if (m_kind != expression_kind) {
            return *this;
        } else {
            // Follow the operand dtype chain to get the storage dtype
            const dtype* dt = &static_cast<const extended_expression_dtype *>(m_extended)->get_operand_dtype();
            while (dt->kind() == expression_kind) {
                dt = &static_cast<const extended_expression_dtype *>(dt->m_extended)->get_operand_dtype();
            }
            return *dt;
        }
    }

    /**
     * The type number is an enumeration of data types, starting
     * at 0, with one value for each unique data type. This is
     * inspired by the approach in NumPy, and the intention is
     * to have the default
     */
    type_id_t type_id() const {
        return (type_id_t)m_type_id;
    }

    /** The 'kind' of the dtype (int, uint, float, etc) */
    dtype_kind_t kind() const {
        return (dtype_kind_t)m_kind;
    }

    /*
     * Return a comparison kernel that can perform the requested single comparison on
     * data of this dtype
     *
     * @param compare_id the identifier of the comparison
     */
    void get_single_compare_kernel(single_compare_kernel_instance& out_kernel) const;

    /** The alignment of the dtype */
    size_t alignment() const {
        return m_alignment;
    }

    /** Increments the offset as much as is needed so it is aligned appropriately */
    size_t inc_to_alignment(size_t offset) const {
        return ::dynd::inc_to_alignment(offset, m_alignment);
    }

    /** Increments the pointer as much as is needed so it is aligned appropriately */
    char *inc_to_alignment(char *ptr) const {
        return ::dynd::inc_to_alignment(ptr, m_alignment);
    }

    /** Increments the pointer as much as is needed so it is aligned appropriately */
    const char *apply_alignment(const char *ptr) const {
        return reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(ptr) + m_alignment - 1) & (-m_alignment));
    }

    /** The element size of the dtype */
    size_t element_size() const {
        return m_element_size;
    }

    /** For string dtypes, their encoding */
    string_encoding_t string_encoding() const {
        if (m_kind == string_kind) {
            return static_cast<const extended_string_dtype *>(m_extended)->encoding();
        } else {
            throw std::runtime_error("Can only get the string encoding from string_kind types");
        }
    }

    dtype_memory_management_t get_memory_management() const {
        if (m_extended != NULL) {
            return m_extended->get_memory_management();
        } else {
            return pod_memory_management;
        }
    }

    /**
     * Returns a const pointer to the extended_dtype object which
     * contains information about the dtype, or NULL if no extended
     * dtype information exists. The returned pointer is only valid during
     * the lifetime of the dtype.
     */
    const extended_dtype* extended() const {
        return m_extended;
    }

    /**
     * print data interpreted as a single value of this dtype
     *
     * @param o         the std::ostream to print to
     * @param data      pointer to the data element to print
     * @param metadata  pointer to the ndobject metadata for the data element
     */
    void print_element(std::ostream& o, const char *data, const char *metadata) const;

    friend /* TODO: DYND_CONSTEXPR*/ dtype detail::internal_make_raw_dtype(char type_id, char kind, intptr_t element_size, char alignment);
    friend std::ostream& operator<<(std::ostream& o, const dtype& rhs);
};

// Convenience function which makes a dtype object from a template parameter
template<class T>
dtype make_dtype()
{
    return dtype(type_id_of<T>::value);
}

/**
 * A static array of the builtin dtypes and void.
 * If code is specialized just for a builtin type, like int, it can use
 * static_builtin_dtypes[type_id_of<int>::value] as a fast
 * way to get a const reference to its dtype.
 */
extern const dtype static_builtin_dtypes[builtin_type_id_count + 1];

std::ostream& operator<<(std::ostream& o, const dtype& rhs);
/** Prints raw bytes as hexadecimal */
void hexadecimal_print(std::ostream& o, char value);
void hexadecimal_print(std::ostream& o, unsigned char value);
void hexadecimal_print(std::ostream& o, unsigned short value);
void hexadecimal_print(std::ostream& o, unsigned int value);
void hexadecimal_print(std::ostream& o, unsigned long value);
void hexadecimal_print(std::ostream& o, unsigned long long value);
void hexadecimal_print(std::ostream& o, const char *data, intptr_t element_size);

} // namespace dynd

#endif // _DYND__DTYPE_HPP_