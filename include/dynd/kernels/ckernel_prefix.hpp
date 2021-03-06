//
// Copyright (C) 2011-14 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <dynd/config.hpp>
#include <dynd/diagnostics.hpp>

namespace dynd {

struct ckernel_prefix;

typedef void (*expr_single_t)(char *dst, char **src, ckernel_prefix *self);
typedef void (*expr_strided_t)(char *dst, intptr_t dst_stride, char **src,
                               const intptr_t *src_stride, size_t count,
                               ckernel_prefix *self);
typedef int (*expr_predicate_t)(const char *const *src, ckernel_prefix *self);

/**
 * Definition for kernel request parameters.
 */
enum {
  /** Kernel function expr_single_t, "(T1, T2, ...) -> R" */
  kernel_request_single = 0,
  /** Kernel function expr_strided_t, "(T1, T2, ...) -> R" */
  kernel_request_strided = 1,
  /** Kernel function expr_predicate_t, "(T1, T2, ...) -> bool" */
  kernel_request_predicate = 2,

  /** Y */
  kernel_request_host = 8,
  /** Y */
  kernel_request_cuda_device = 16
};
typedef uint32_t kernel_request_t;

/**
 * This is the struct which begins the memory layout
 * of all ckernels. First comes the function pointer,
 * which has a context-specific prototype, such as
 * `expr_single_t`, and then comes the
 * destructor.
 *
 * The ckernel is defined in terms of a C ABI definition,
 * and must satisfy alignments, movability, and
 * concurrency requirements to be valid. See the
 * document
 */
struct ckernel_prefix {
  typedef void (*destructor_fn_t)(ckernel_prefix *);

  void *function;
  destructor_fn_t destructor;

  /**
   * To help with generic code a bit, structs which
   * begin with a ckernel_prefix can define this
   * base() function which returns that ckernel_prefix.
   */
  DYND_CUDA_HOST_DEVICE ckernel_prefix &base() { return *this; }

  /**
   * Aligns an offset as required by ckernels.
   */
  DYND_CUDA_HOST_DEVICE static size_t align_offset(size_t offset)
  {
    return (offset + size_t(7)) & ~size_t(7);
  }

  /**
   * Call to get the kernel function pointer, whose type
   * must be known by the context.
   *
   *      kdp->get_function<expr_single_t>()
   */
  template <typename T>
  DYND_CUDA_HOST_DEVICE T get_function() const
  {
    return reinterpret_cast<T>(function);
  }

  template <typename T>
  DYND_CUDA_HOST_DEVICE void set_function(T fnptr)
  {
    function = reinterpret_cast<void *>(fnptr);
  }

  DYND_CUDA_HOST_DEVICE void set_expr_function(kernel_request_t kernreq,
                                               expr_single_t single,
                                               expr_strided_t strided)
  {
    if (kernreq == kernel_request_single) {
      function = reinterpret_cast<void *>(single);
    } else if (kernreq == kernel_request_strided) {
      function = reinterpret_cast<void *>(strided);
    } else {
      DYND_HOST_THROW(std::runtime_error, "unrecognized dynd kernel request " +
                                              std::to_string(kernreq));
    }
  }

  template <class T>
  void set_expr_function(kernel_request_t kernreq)
  {
    set_expr_function(kernreq, &T::single, &T::strided);
  }

  /**
   * Calls the destructor of the ckernel if it is
   * non-NULL.
   */
  DYND_CUDA_HOST_DEVICE void destroy()
  {
    if (destructor != NULL) {
      destructor(this);
    }
  }

  /**
   * Returns the pointer to a child ckernel at the provided
   * offset.
   */
  DYND_CUDA_HOST_DEVICE ckernel_prefix *get_child_ckernel(intptr_t offset)
  {
    return reinterpret_cast<ckernel_prefix *>(
        reinterpret_cast<char *>(this) + ckernel_prefix::align_offset(offset));
  }

  /**
   * If the provided offset is non-zero, destroys
   * a ckernel at the given offset from `this`.
   */
  void destroy_child_ckernel(size_t offset)
  {
    if (offset != 0) {
      ckernel_prefix *child = get_child_ckernel(offset);
      child->destroy();
    }
  }
};

} // namespace dynd
