#include "mm.h"
#include "mb_node.h"
#include <stdlib.h>
#include <assert.h>
/*OVS HEAD FILE*/
#include "list.h"
#include "util.h"

static struct ovs_list allocators = OVS_LIST_INITIALIZER(&allocators);

static struct mem_op * find_allocator(int type)
{
    struct allocator *alloc;
    int found = 0;

    LIST_FOR_EACH(alloc, list, &allocators) {
        if(alloc->op->type == type ) {
            found = 1;
            break;
        }
    }

    if(found)
        return alloc->op;
    else
        return NULL;
}

int add_allocator(struct mem_op *op)
{
    const struct mem_op *find = find_allocator(op->type);
    if(find) {
        printf("Type %d exist\n", find->type);
        return -1;
    }


    struct allocator *alloc = (struct allocator *)calloc(1, sizeof(struct allocator));
    if(!alloc) {
        printf("out of mem\n");
        return -1;
    }

    list_init(&alloc->list);
    alloc->op = op;
    list_push_back(&alloc->list, &allocators);
    return 0;
}


int mm_init(struct mm *m, int type)
{
    memset(&m->ms, 0, sizeof(m->ms));
    m->op = find_allocator(type);
    assert(m->op);
    int ret = 0;

    if(m->op->init) {
        m->op->priv = calloc(m->op->priv_size, 1);
        ret = m->op->init(m->op->priv);
    }
    return ret;
}

int mm_uinit(struct mm *m)
{
    int ret = 0;
    if(m->op->uinit) {
        ret = m->op->uinit(m->op->priv);
    }

    if(m->op->priv_size) {
        free(m->op->priv);
    }

    return ret;
}


void *alloc_node(struct mm *m, uint32_t node_num, uint32_t level)
{
    m->ms.mem += node_num * NODE_SIZE;
    m->ms.node += node_num;
    m->ms.lmem[level] += node_num * NODE_SIZE;
    m->ms.lnode[level] += node_num;

    return m->op->alloc_node(m, node_num, level);
}


void dealloc_node(struct mm *m, uint32_t node_num, uint32_t level, void *p)
{
    m->ms.mem -= node_num * NODE_SIZE;
    m->ms.node -= node_num;
    m->ms.lmem[level] -= node_num * NODE_SIZE;
    m->ms.lnode[level] -= node_num;

    m->op->dealloc_node(m, node_num, level, p);
}

void mm_profile(struct mm *m) 
{
    int i; 

    printf("Total memory %dB %.2fKB %.2fMB\n",
            m->ms.mem, (double)(m->ms.mem)/1024.0,
            (double)(m->ms.mem)/(1024*1024.0));

    printf("Total nodes %d\n",
            m->ms.node);

    for(i = 0; i < MAX_LEVEL; i++ ) {
        if(m->ms.lmem[i] != 0) {
            printf("level %d with memory %dB %.2fKB %.2fMB\n",
                    i, m->ms.lmem[i], (double)(m->ms.lmem[i])/1024.0,
                    (double)(m->ms.lmem[i])/(1024*1024.0));
            printf("level %d with nodes %d\n",
                    i, m->ms.lnode[i]);
        }
        else
            break;
    }

    if(m->op->mm_profile) {
        m->op->mm_profile(m);
    }
}





/*
 *
 * A simple memory management implementation
 *
 */


static void* alloc_simple(struct mm *m OVS_UNUSED, uint32_t node_num, uint32_t level OVS_UNUSED)
{
    void *ret;
    ret = xmalloc(node_num * NODE_SIZE);
    memset(ret, 0, node_num * NODE_SIZE);
    return ret; 
}

static void dealloc_simple(struct mm *m OVS_UNUSED, uint32_t node_num OVS_UNUSED, uint32_t level OVS_UNUSED, void *p)
{
    free(p);
}

static struct mem_op simple = {
    .type = MEM_ALLOC_SIMPLE,
    .alloc_node = alloc_simple,
    .dealloc_node = dealloc_simple,
};

ADD_ALLOCATOR(simple);


