/*
 * ring_buffer.h
 *
 *  Created on: Jan 2, 2016
 *      Author: Nguyen Dinh Trung Truc
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Modify the ring buffer's size, should be a multiple of 4 */
#define MAX_SIZE_RB 240

typedef struct rb_handle_t
{
    uint16_t head;
    uint16_t tail;
    uint16_t size;
    uint16_t max_size;

    uint8_t buffer[MAX_SIZE_RB];
} rb_handle_t;

/**
 * @brief   Initialize ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[in]   size, max size of the ring buffer
 *
 * @return
 *      1: success
 *      0: failure
 */
uint8_t rb_init(rb_handle_t *rb_data, uint16_t size);

/**
 * @brief   add data to the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[in]   buf, data to add
 * @param[in]   size, size of data
 *
 * @return
 *      1: success
 *      0: failure
 */
uint8_t rb_add_data(rb_handle_t *rb_data, uint8_t *buf, uint16_t size);

/**
 * @brief   get data from the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 * @param[out]   data, data received
 * @param[in]   size, size of data to receive
 *
 * @return  real size of received data
 */
uint16_t rb_get_data(rb_handle_t *rb_data, uint8_t *data, uint16_t size);

/**
 * @brief   get free size of the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 *
 * @return  free size of the ring buffer
 */
uint16_t rb_get_free_size(rb_handle_t *rb_data);

/**
 * @brief   get size of the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 *
 * @return  size of the ring buffer
 */
uint16_t rb_get_size(rb_handle_t *rb_data);

/**
 * @brief   clear the ring buffer
 * @param[in]   rb_data, a ring buffer handle
 */
void rb_clear(rb_handle_t *rb_data);

#endif /* RING_BUFFER_H_ */
