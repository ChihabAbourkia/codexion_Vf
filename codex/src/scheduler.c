#include "codex.h"

static  void swap(t_request *a, t_request *b)
{
    t_request tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

long get_priority(t_coder *coder)
{
    long priority;
    pthread_mutex_lock(&coder->sim->state_mutex);
    if(coder->sim->params.scheduler == EDF)
    {
        priority = coder->last_compile_start + coder->sim->params.time_to_burnout;
    }
    else
    {
        priority = current_time_ms();
    }
    pthread_mutex_unlock(&coder->sim->state_mutex);
    return priority;    
}

void queue_push(t_queue *queue, t_coder *coder, long priority)
{
    queue->requests[queue->size].coder = coder;
    queue->requests[queue->size].priority = priority;
    queue->size++;
    if( queue->size == 2 && queue->requests[0].priority > queue->requests[1].priority)
        swap(&queue->requests[0], &queue->requests[1]);
}
void queue_pop(t_queue *queue)
{
    if(queue->size == 2)
    {
        queue->requests[0] = queue->requests[1];
    }
    if(queue->size)
    {
        queue->size--;
    }
}
int queue_front(t_queue *queue)
{
    if(queue->size == 0)
        return -1;
    return queue->requests[0].coder->id;
}