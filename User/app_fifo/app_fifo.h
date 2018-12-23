#ifndef __APP_FIFO_H_
#define __APP_FIFO_H_

#include <stdint.h>

#define FIFO_SUCCESS                           (0)  ///< Successful command
#define FIFO_ERROR                             (1)  ///< Successful command

typedef struct
{
    uint8_t *          p_buf;           /**< Pointer to FIFO buffer memory.                      */
    uint16_t           buf_size_mask;   /**< Read/write index mask. Also used for size checking. */
    volatile uint32_t  read_pos;        /**< Next read position in the FIFO buffer.              */
    volatile uint32_t  write_pos;       /**< Next write position in the FIFO buffer.             */
} app_fifo_t;

/**@brief Macro for checking if an integer is a power of two.
 *
 * @param[in]   A   Number to be tested.
 *
 * @return      true if value is power of two.
 * @return      false if value not power of two.
 */
#define IS_POWER_OF_TWO(A) ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )

uint32_t app_fifo_init(app_fifo_t * p_fifo, uint8_t * p_buf, uint16_t buf_size);
uint32_t app_fifo_put(app_fifo_t * p_fifo, uint8_t byte);
uint32_t app_fifo_get(app_fifo_t * p_fifo, uint8_t * p_byte);
uint32_t app_fifo_write(app_fifo_t * p_fifo, uint8_t const * p_byte_array, uint32_t a_size);

#endif
