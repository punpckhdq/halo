/*
CIRCULAR_QUEUE.H

header included in hcex build.
*/

#ifndef __CIRCULAR_QUEUE_H
#define __CIRCULAR_QUEUE_H
#pragma once

/* ---------- prototypes/CIRCULAR_QUEUE.C */

void circular_queue_reset(struct circular_queue *queue);
struct circular_queue *circular_queue_new(char *name, long buffer_size);
void circular_queue_delete(struct circular_queue *queue);
long circular_queue_size(struct circular_queue *queue);
long circular_queue_free_space(struct circular_queue *queue);
boolean circular_queue_queue_data(struct circular_queue *queue, void *data, long data_size);
boolean circular_queue_dequeue_data(struct circular_queue *queue, void *data, long data_size, boolean advance);

#endif // __CIRCULAR_QUEUE_H
