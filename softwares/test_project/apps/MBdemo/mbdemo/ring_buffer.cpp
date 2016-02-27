/*
 * ring_buffer.c
 *
 *  Created on: Jan 2, 2016
 *      Author: Nguyen Dinh Trung Truc
 */
#include "ring_buffer.h"

/*----------------------------------------------------------------------------*/
uint8_t rb_init(rb_handle_t *rb_data, uint16_t size)
{
    uint16_t i;

    if (size > MAX_SIZE_RB) return 0;

    for (i = 0; i < size; i++)
    {
        rb_data->buffer[i] = 0;
    }

    rb_data->head = 0;
    rb_data->tail = 0;
    rb_data->size = 0;
    rb_data->max_size = size;

    return 1;

}

/*----------------------------------------------------------------------------*/
uint8_t rb_add_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size)
{
    uint16_t i;

    if (rb_data->size + size <= rb_data->max_size)
    {
        for (i = 0; i < size; i++)
        {
            rb_data->buffer[rb_data->tail] = buf[i];
            rb_data->tail = (rb_data->tail + 1) % rb_data->max_size;
            rb_data->size++;
        }
        return 1;
    }
    else return 0;
}

/*----------------------------------------------------------------------------*/
uint16_t rb_get_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size)
{
    uint16_t i;

    size = (size < rb_data->size)? size : rb_data->size;

    for (i = 0; i < size; i++)
    {
        buf[i] = rb_data->buffer[rb_data->head];
        rb_data->head = (rb_data->head + 1) % rb_data->max_size;
        rb_data->size--;
    }

    return size;
}

/*----------------------------------------------------------------------------*/
uint16_t rb_get_free_size(rb_handle_t *rb_data)
{
    return rb_data->max_size - rb_data->size;
}

/*----------------------------------------------------------------------------*/
uint16_t rb_get_size(rb_handle_t *rb_data)
{
    return rb_data->size;
}

/*----------------------------------------------------------------------------*/
void rb_clear(rb_handle_t *rb_data)
{
    rb_data->size = 0;
    rb_data->head = 0;
    rb_data->tail = 0;
}
