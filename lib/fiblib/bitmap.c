#include "bitmap.h"

#include "util.h"

int prefix_exist_func(struct mb_node *node, 
        uint8_t stride OVS_UNUSED, uint8_t pos, uint8_t type, void *data OVS_UNUSED)
{
    if(type == LEAF_NODE) {
        if (!(test_bitmap(node->internal , pos))) {
            return 0;
        }
    }
    else {
        if(!test_bitmap(node->external,pos)) {
            return 0;
        }
    }

    return TRAVERSE_CONT;
}

int update_nodes(struct mm *mm, struct trace *t, int total)
{
    int i;
    int node_need_to_del = 0;
    for(i=total;i >= 0;i--){
        if(i==total){
            reduce_rule(mm, t[i].node, 
                    count_ones((t[i].node)->internal, t[i].pos) + 1, i);
            clear_bitmap(&(t[i].node)->internal, (t[i].pos));
            if((t[i].node)->internal == 0 && (t[i].node)->external == 0)
            {
                node_need_to_del = 1;
            }
        }
        else{
            if(node_need_to_del){
                reduce_child(mm, t[i].node, 
                        count_ones((t[i].node)->external, t[i].pos), i);
                clear_bitmap(&(t[i].node)->external, (t[i].pos));
            }
            if((t[i].node)->internal == 0 && (t[i].node)->external == 0){
                node_need_to_del = 1;
            }
            else{
                node_need_to_del = 0;
            }
        }
    }
    return node_need_to_del;
}

