//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/type.hpp>
#include <dynd/kernels/option_kernels.hpp>
#include <dynd/kernels/assignment_kernels.hpp>
#include <dynd/types/option_type.hpp>
#include <dynd/types/typevar_type.hpp>

using namespace std;
using namespace dynd;

namespace {

template<class T>
struct is_avail;

template<class T>
struct assign_na;

//////////////////////////////////////
// option[bool]
// NA is 2

template<>
struct is_avail<dynd_bool> {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
        // Available if the value is 0 or 1
        *dst = **reinterpret_cast<unsigned char **>(src) <= 1;
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        // Available if the value is 0 or 1
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
            *dst = *reinterpret_cast<unsigned char *>(src) <= 1;
            dst += dst_stride;
            src0 += src0_stride;
        }
    }
};

template<>
struct assign_na<dynd_bool> {
    static void single(char *dst, char ** DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        *dst = 2;
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        if (dst_stride == 1) {
            memset(dst, 2, count);
        } else {
            for (size_t i = 0; i != count; ++i, dst += dst_stride) {
                *dst = 2;
            }
        }
    }
};

//////////////////////////////////////
// option[T] for signed integer T
// NA is the smallest negative value

template<class T>
struct int_is_avail {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
        *dst = **reinterpret_cast<T **>(src) !=
               numeric_limits<T>::min();
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
            *dst = *reinterpret_cast<T *>(src0) != numeric_limits<T>::min();
            dst += dst_stride;
            src0 += src0_stride;
        }
    }
};

template <>
struct is_avail<int8_t> : public int_is_avail<int8_t> {};
template <>
struct is_avail<int16_t> : public int_is_avail<int16_t> {};
template <>
struct is_avail<int32_t> : public int_is_avail<int32_t> {};
template <>
struct is_avail<int64_t> : public int_is_avail<int64_t> {};
template <>
struct is_avail<dynd_int128> : public int_is_avail<dynd_int128> {};

template<class T>
struct int_assign_na {
    static void single(char *dst, char **DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        *reinterpret_cast<T *>(dst) = numeric_limits<T>::min();
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        for (size_t i = 0; i != count; ++i, dst += dst_stride) {
            *reinterpret_cast<T *>(dst) = numeric_limits<T>::min();
        }
    }
};

template <>
struct assign_na<int8_t> : public int_assign_na<int8_t> {};
template <>
struct assign_na<int16_t> : public int_assign_na<int16_t> {};
template <>
struct assign_na<int32_t> : public int_assign_na<int32_t> {};
template <>
struct assign_na<int64_t> : public int_assign_na<int64_t> {};
template <>
struct assign_na<dynd_int128> : public int_assign_na<dynd_int128> {};

//////////////////////////////////////
// option[float]
// NA is 0x7f8007a2
// Special rule adopted from R: Any NaN is NA

template<>
struct is_avail<float> {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
      *dst = DYND_ISNAN(**reinterpret_cast<float **>(src)) == 0;
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
          *dst = DYND_ISNAN(*reinterpret_cast<float *>(src0)) == 0;
          dst += dst_stride;
          src0 += src0_stride;
        }
    }
};

template<>
struct assign_na<float> {
    static void single(char *dst, char ** DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        *reinterpret_cast<uint32_t *>(dst) = DYND_FLOAT32_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        for (size_t i = 0; i != count; ++i, dst += dst_stride) {
            *reinterpret_cast<uint32_t *>(dst) = DYND_FLOAT32_NA_AS_UINT;
        }
    }
};

//////////////////////////////////////
// option[double]
// NA is 0x7ff00000000007a2ULL
// Special rule adopted from R: Any NaN is NA

template<>
struct is_avail<double> {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
      *dst = DYND_ISNAN(**reinterpret_cast<double **>(src)) == 0;
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
          *dst = DYND_ISNAN(*reinterpret_cast<double *>(src0)) == 0;
          dst += dst_stride;
          src0 += src0_stride;
        }
    }
};

template<>
struct assign_na<double> {
    static void single(char *dst, char **DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        *reinterpret_cast<uint64_t *>(dst) = DYND_FLOAT64_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        for (size_t i = 0; i != count; ++i, dst += dst_stride) {
            *reinterpret_cast<uint64_t *>(dst) = DYND_FLOAT64_NA_AS_UINT;
        }
    }
};

//////////////////////////////////////
// option[complex[float]]
// NA is two float NAs

template<>
struct is_avail<dynd_complex<float> > {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
        *dst = (*reinterpret_cast<uint32_t **>(src))[0] !=
                   DYND_FLOAT32_NA_AS_UINT &&
               (*reinterpret_cast<uint32_t **>(src))[1] !=
                   DYND_FLOAT32_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
            *dst = reinterpret_cast<uint32_t *>(src0)[0] !=
                       DYND_FLOAT32_NA_AS_UINT &&
                   reinterpret_cast<uint32_t *>(src0)[1] !=
                       DYND_FLOAT32_NA_AS_UINT;
            dst += dst_stride;
            src0 += src0_stride;
        }
    }
};

template<>
struct assign_na<dynd_complex<float> > {
    static void single(char *dst, char **DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        reinterpret_cast<uint32_t *>(dst)[0] = DYND_FLOAT32_NA_AS_UINT;
        reinterpret_cast<uint32_t *>(dst)[1] = DYND_FLOAT32_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        for (size_t i = 0; i != count; ++i, dst += dst_stride) {
            reinterpret_cast<uint32_t *>(dst)[0] = DYND_FLOAT32_NA_AS_UINT;
            reinterpret_cast<uint32_t *>(dst)[1] = DYND_FLOAT32_NA_AS_UINT;
        }
    }
};

//////////////////////////////////////
// option[complex[double]]
// NA is two double NAs

template<>
struct is_avail<dynd_complex<double> > {
    static void single(char *dst, char **src,
                       ckernel_prefix *DYND_UNUSED(self))
    {
        *dst = (*reinterpret_cast<uint64_t **>(src))[0] !=
                   DYND_FLOAT64_NA_AS_UINT &&
               (*reinterpret_cast<uint64_t **>(src))[1] !=
                   DYND_FLOAT64_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride, char **src,
                        const intptr_t *src_stride, size_t count,
                        ckernel_prefix *DYND_UNUSED(self))
    {
        // Available if the value is 0 or 1
        char *src0 = src[0];
        intptr_t src0_stride = src_stride[0];
        for (size_t i = 0; i != count; ++i) {
            *dst = reinterpret_cast<uint64_t *>(src0)[0] !=
                       DYND_FLOAT64_NA_AS_UINT &&
                   reinterpret_cast<uint64_t *>(src0)[1] !=
                       DYND_FLOAT64_NA_AS_UINT;
            dst += dst_stride;
            src0 += src0_stride;
        }
    }
};

template<>
struct assign_na<dynd_complex<double> > {
    static void single(char *dst, char **DYND_UNUSED(src),
                                ckernel_prefix *DYND_UNUSED(strided))
    {
        reinterpret_cast<uint64_t *>(dst)[0] = DYND_FLOAT64_NA_AS_UINT;
        reinterpret_cast<uint64_t *>(dst)[1] = DYND_FLOAT64_NA_AS_UINT;
    }

    static void strided(char *dst, intptr_t dst_stride,
                                  char **DYND_UNUSED(src),
                                  const intptr_t *DYND_UNUSED(src_stride),
                                  size_t count,
                                  ckernel_prefix *DYND_UNUSED(strided))
    {
        for (size_t i = 0; i != count; ++i, dst += dst_stride) {
            reinterpret_cast<uint64_t *>(dst)[0] = DYND_FLOAT64_NA_AS_UINT;
            reinterpret_cast<uint64_t *>(dst)[1] = DYND_FLOAT64_NA_AS_UINT;
        }
    }
};

//////////////////////////////////////
// option[pointer[T]]

template <class T>
struct is_avail<T *> {
    static void single(char *DYND_UNUSED(dst), char **DYND_UNUSED(src),
                       ckernel_prefix *DYND_UNUSED(strided))
    {
        throw std::runtime_error("is_avail for pointers is not yet implemented");
    }

    static void strided(char *DYND_UNUSED(dst), intptr_t DYND_UNUSED(dst_stride),
                        char **DYND_UNUSED(src),
                        const intptr_t *DYND_UNUSED(src_stride),
                        size_t DYND_UNUSED(count),
                        ckernel_prefix *DYND_UNUSED(strided))
    {
        throw std::runtime_error("is_avail for pointers is not yet implemented");
    }
};

template <class T>
struct assign_na<T *> {
    static void single(char *DYND_UNUSED(dst), char **DYND_UNUSED(src),
                       ckernel_prefix *DYND_UNUSED(strided))
    {
        throw std::runtime_error("assign_na for pointers is not yet implemented");   
    }

    static void strided(char *DYND_UNUSED(dst), intptr_t DYND_UNUSED(dst_stride),
                        char **DYND_UNUSED(src),
                        const intptr_t *DYND_UNUSED(src_stride),
                        size_t DYND_UNUSED(count),
                        ckernel_prefix *DYND_UNUSED(strided))
    {
        throw std::runtime_error("assign_na for pointers is not yet implemented");   
    }
};

template<typename T>
struct nafunc {
    typedef typename std::remove_pointer<T>::type nafunc_type;

    static intptr_t instantiate_is_avail(
        const arrfunc_type_data *DYND_UNUSED(self),
        const arrfunc_type *DYND_UNUSED(af_tp), void *ckb,
        intptr_t ckb_offset, const ndt::type &dst_tp,
        const char *DYND_UNUSED(dst_arrmeta), const ndt::type *src_tp,
        const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
        const eval::eval_context *DYND_UNUSED(ectx),
        const nd::array &DYND_UNUSED(args), const nd::array &DYND_UNUSED(kwds))
    {
      if (src_tp[0].get_type_id() != option_type_id ||
          src_tp[0].extended<option_type>()->get_value_type().get_type_id() !=
              (type_id_t)type_id_of<nafunc_type>::value) {
        stringstream ss;
        ss << "Expected source type ?" << ndt::make_type<nafunc_type>()
           << ", got " << src_tp[0];
        throw type_error(ss.str());
      }
      if (dst_tp.get_type_id() != bool_type_id) {
        stringstream ss;
        ss << "Expected destination type bool, got " << dst_tp;
        throw type_error(ss.str());
      }
      ckernel_prefix *ckp = reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)->alloc_ck_leaf<ckernel_prefix>(ckb_offset);
      ckp->set_expr_function< ::is_avail<T> >(kernreq);
      return ckb_offset;
    }

    static int resolve_is_avail_dst_type(
        const arrfunc_type_data *DYND_UNUSED(self),
        const arrfunc_type *DYND_UNUSED(af_tp), intptr_t DYND_UNUSED(nsrc),
        const ndt::type *DYND_UNUSED(src_tp), int DYND_UNUSED(throw_on_error),
        ndt::type &out_dst_tp, const nd::array &DYND_UNUSED(args),
        const nd::array &DYND_UNUSED(kwds))
    {
      out_dst_tp = ndt::make_type<dynd_bool>();
      return 1;
    }

    static intptr_t instantiate_assign_na(
        const arrfunc_type_data *DYND_UNUSED(self),
        const arrfunc_type *DYND_UNUSED(af_tp), void *ckb,
        intptr_t ckb_offset, const ndt::type &dst_tp,
        const char *DYND_UNUSED(dst_arrmeta),
        const ndt::type *DYND_UNUSED(src_tp),
        const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
        const eval::eval_context *DYND_UNUSED(ectx),
        const nd::array &DYND_UNUSED(args), const nd::array &DYND_UNUSED(kwds))
    {
      if (dst_tp.get_type_id() != option_type_id ||
          dst_tp.extended<option_type>()->get_value_type().get_type_id() !=
              (type_id_t)type_id_of<nafunc_type>::value) {
        stringstream ss;
        ss << "Expected dst type " << ndt::make_type<nafunc_type>() << ", got "
           << dst_tp;
        throw type_error(ss.str());
      }
      ckernel_prefix *ckp = reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)->alloc_ck_leaf<ckernel_prefix>(ckb_offset);
      ckp->set_expr_function< ::assign_na<T> >(kernreq);
      return ckb_offset;
    }

    static nd::array get()
    {
      nd::array naf = nd::empty(option_type::make_nafunc_type());
      arrfunc_type_data *is_avail =
          reinterpret_cast<arrfunc_type_data *>(naf.get_ndo()->m_data_pointer);
      arrfunc_type_data *assign_na = is_avail + 1;

      is_avail->instantiate = &nafunc::instantiate_is_avail;
      is_avail->resolve_dst_type = &nafunc::resolve_is_avail_dst_type;
      assign_na->instantiate = &nafunc::instantiate_assign_na;
      naf.flag_as_immutable();
      return naf;
    }
};

} // anonymous namespace

intptr_t kernels::fixed_dim_is_avail_ck::instantiate(
    const arrfunc_type_data *DYND_UNUSED(self),
    const arrfunc_type *DYND_UNUSED(af_tp), void *ckb,
    intptr_t ckb_offset, const ndt::type &DYND_UNUSED(dst_tp),
    const char *DYND_UNUSED(dst_arrmeta), const ndt::type *src_tp,
    const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
    const eval::eval_context *DYND_UNUSED(ectx),
    const nd::array &DYND_UNUSED(args), const nd::array &DYND_UNUSED(kwds))
{
  ckernel_prefix *ckp = reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)->alloc_ck_leaf<ckernel_prefix>(ckb_offset);
  switch (src_tp->get_dtype().get_type_id()) {
  case bool_type_id:
    ckp->set_expr_function<is_avail<dynd_bool> >(kernreq);
    break;
  case int8_type_id:
    ckp->set_expr_function<is_avail<int8_t> >(kernreq);
    break;
  case int16_type_id:
    ckp->set_expr_function<is_avail<int16_t> >(kernreq);
    break;
  case int32_type_id:
    ckp->set_expr_function<is_avail<int32_t> >(kernreq);
    break;
  case int64_type_id:
    ckp->set_expr_function<is_avail<int64_t> >(kernreq);
    break;
  case int128_type_id:
    ckp->set_expr_function<is_avail<dynd_int128> >(kernreq);
    break;
  case float32_type_id:
    ckp->set_expr_function<is_avail<float> >(kernreq);
    break;
  case float64_type_id:
    ckp->set_expr_function<is_avail<double> >(kernreq);
    break;
  case complex_float32_type_id:
    ckp->set_expr_function<is_avail<dynd_complex<float> > >(kernreq);
    break;
  case complex_float64_type_id:
    ckp->set_expr_function<is_avail<dynd_complex<double> > >(kernreq);
    break;
  default:
    throw type_error("fixed_dim_is_avail: expected built-in type");
    break;
  }
  return ckb_offset;
}

intptr_t kernels::fixed_dim_assign_na_ck::instantiate(
    const arrfunc_type_data *DYND_UNUSED(self),
    const arrfunc_type *DYND_UNUSED(af_tp), void *ckb,
    intptr_t ckb_offset, const ndt::type &dst_tp,
    const char *DYND_UNUSED(dst_arrmeta), const ndt::type *DYND_UNUSED(src_tp),
    const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
    const eval::eval_context *DYND_UNUSED(ectx),
    const nd::array &DYND_UNUSED(args), const nd::array &DYND_UNUSED(kwds))
{
  ckernel_prefix *ckp = reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)->alloc_ck_leaf<ckernel_prefix>(ckb_offset);
  switch (dst_tp.get_dtype().get_type_id()) {
  case bool_type_id:
    ckp->set_expr_function<assign_na<dynd_bool> >(kernreq);
    break;
  case int8_type_id:
    ckp->set_expr_function<assign_na<int8_t> >(kernreq);
    break;
  case int16_type_id:
    ckp->set_expr_function<assign_na<int16_t> >(kernreq);
    break;
  case int32_type_id:
    ckp->set_expr_function<assign_na<int32_t> >(kernreq);
    break;
  case int64_type_id:
    ckp->set_expr_function<assign_na<int64_t> >(kernreq);
    break;
  case int128_type_id:
    ckp->set_expr_function<assign_na<dynd_int128> >(kernreq);
    break;
  case float32_type_id:
    ckp->set_expr_function<assign_na<float> >(kernreq);
    break;
  case float64_type_id:
    ckp->set_expr_function<assign_na<double> >(kernreq);
    break;
  case complex_float32_type_id:
    ckp->set_expr_function<assign_na<dynd_complex<float> > >(kernreq);
    break;
  case complex_float64_type_id:
    ckp->set_expr_function<assign_na<dynd_complex<double> > >(kernreq);
    break;
  default:
    throw type_error("fixed_dim_assign_na: expected built-in type");
    break;
  }
  return ckb_offset;
}

const nd::array &kernels::get_option_builtin_nafunc(type_id_t tid)
{
  static nd::array bna = nafunc<dynd_bool>::get();
  static nd::array i8na = nafunc<int8_t>::get();
  static nd::array i16na = nafunc<int16_t>::get();
  static nd::array i32na = nafunc<int32_t>::get();
  static nd::array i64na = nafunc<int64_t>::get();
  static nd::array i128na = nafunc<dynd_int128>::get();
  static nd::array f32na = nafunc<float>::get();
  static nd::array f64na = nafunc<double>::get();
  static nd::array cf32na = nafunc<dynd_complex<float> >::get();
  static nd::array cf64na = nafunc<dynd_complex<double> >::get();
  static nd::array nullarr;
  switch (tid) {
  case bool_type_id:
    return bna;
  case int8_type_id:
    return i8na;
  case int16_type_id:
    return i16na;
  case int32_type_id:
    return i32na;
  case int64_type_id:
    return i64na;
  case int128_type_id:
    return i128na;
  case float32_type_id:
    return f32na;
  case float64_type_id:
    return f64na;
  case complex_float32_type_id:
    return cf32na;
  case complex_float64_type_id:
    return cf64na;
  default:
    return nullarr;
  }
}

const nd::array &kernels::get_option_builtin_pointer_nafunc(type_id_t tid)
{
  static nd::array bna = nafunc<dynd_bool *>::get();
  static nd::array i8na = nafunc<int8_t *>::get();
  static nd::array i16na = nafunc<int16_t *>::get();
  static nd::array i32na = nafunc<int32_t *>::get();
  static nd::array i64na = nafunc<int64_t *>::get();
  static nd::array i128na = nafunc<dynd_int128 *>::get();
  static nd::array f32na = nafunc<float *>::get();
  static nd::array f64na = nafunc<double *>::get();
  static nd::array cf32na = nafunc<dynd_complex<float> *>::get();
  static nd::array cf64na = nafunc<dynd_complex<double> *>::get();
  static nd::array nullarr;
  switch (tid) {
  case bool_type_id:
    return bna;
  case int8_type_id:
    return i8na;
  case int16_type_id:
    return i16na;
  case int32_type_id:
    return i32na;
  case int64_type_id:
    return i64na;
  case int128_type_id:
    return i128na;
  case float32_type_id:
    return f32na;
  case float64_type_id:
    return f64na;
  case complex_float32_type_id:
    return cf32na;
  case complex_float64_type_id:
    return cf64na;
  default:
    return nullarr;
  }
}
