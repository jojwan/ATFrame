#include "app_fifo.h"
#include <stdio.h>

uint32_t app_fifo_init(app_fifo_t * p_fifo, uint8_t * p_buf, uint16_t buf_size)
{
    // Check buffer for null pointer.
    if (p_buf == NULL)
    {
        return FIFO_ERROR;
    }

    //必须是2的倍数
    // Check that the buffer size is a power of two.
    if (!IS_POWER_OF_TWO(buf_size))
    {
        return FIFO_ERROR;
    }

    p_fifo->p_buf         = p_buf;
    p_fifo->buf_size_mask = buf_size - 1;
    p_fifo->read_pos      = 0;
    p_fifo->write_pos     = 0;

    return FIFO_SUCCESS;
}

uint32_t app_fifo_put(app_fifo_t * p_fifo, uint8_t byte)
{
    if ( (p_fifo->write_pos+1) == p_fifo->read_pos )
    {
        return FIFO_ERROR;
    }
    p_fifo->p_buf[p_fifo->write_pos & p_fifo->buf_size_mask] = byte;
    p_fifo->write_pos = (++p_fifo->write_pos) & p_fifo->buf_size_mask;
    return FIFO_SUCCESS;
}

uint32_t app_fifo_get(app_fifo_t * p_fifo, uint8_t * p_byte)
{
    while( p_fifo->write_pos == p_fifo->read_pos )
    {
        return FIFO_ERROR;
    }
    
    *p_byte = p_fifo->p_buf[p_fifo->read_pos & p_fifo->buf_size_mask];
    p_fifo->read_pos = (++p_fifo->read_pos) & p_fifo->buf_size_mask;
    return FIFO_SUCCESS;
}

uint32_t app_fifo_write(app_fifo_t * p_fifo, uint8_t const * p_byte_array, uint32_t a_size)
{
    unsigned short i;
    for(i = 0; i < a_size; i ++)
    {
        uint8_t err = app_fifo_put( p_fifo, p_byte_array[i] );
        if ( FIFO_ERROR == err )
            return FIFO_ERROR;
    }
    return FIFO_SUCCESS;
}









