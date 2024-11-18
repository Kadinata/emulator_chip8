#ifndef __STATUS_CODE_H__
#define __STATUS_CODE_H__

typedef enum
{
  STATUS_OK = 0,
  STATUS_ERR_GENERIC,
  STATUS_ERR_NULL_PTR,
  STATUS_ERR_NO_MEMORY,
  STATUS_ERR_FILE_NOT_FOUND,
  STATUS_ERR_INVALID_ADDRESS,
  STATUS_ERR_MEM_OUT_OF_BOUNDS,
  STATUS_ERR_STACK_OVERFLOW,
  STATUS_ERR_STACK_UNDERFLOW,
  STATUS_REQ_EXIT,
} status_code_t;

#define VERIFY_PTR_RETURN_ERROR_IF_NULL(p) \
  if (p == NULL)                           \
  {                                        \
    return STATUS_ERR_NULL_PTR;            \
  }

#define RETURN_STATUS_IF_NOT_OK(status) \
  if (status != STATUS_OK)              \
  {                                     \
    return status;                      \
  }

#endif
