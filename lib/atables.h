#ifndef __A_TABLES_H__
#define __A_TABLES_H__

#include "cmap.h"
#include "flow.h"

//atable type
#define AFLOW_HASH 0
#define AFLOW_TRIE 1

//atable operation
#define AFLOW_HASH_ADD      0
#define AFLOW_HASH_DEL      1
#define AFLOW_HASH_NEW      2
#define AFLOW_HASH_DESTROY  3

//Defined in dpif-netdev.c
//this file can only be included in dpif-netdev.c I guess.
struct netdev_flow_key;
struct dp_netdev_actions;
struct aflow_batch;


struct aflow_hash {
    struct netdev_flow_key mask;
    struct cmap hash;
};


struct aflow_hash_rule {
    struct netdev_flow_key key;
    //FIXME: use RCU lock for actions
    struct dp_netdev_actions *actions;
    struct aflow_batch *batch;

    struct cmap_node node;
};

struct aflow_rule_common {
    struct netdev_flow_key key;
    //FIXME: use RCU lock for actions
    struct dp_netdev_actions *actions;
    struct aflow_batch *batch;
};


#define HASH_RULE_SIZE (sizeof (struct aflow_hash_rule))
#define TRIE_RULE_SIZE 0
#define RULE_COMMON_SIZE (sizeof (struct aflow_rule_common))
#define RULE_MAX ( HASH_RULE_SIZE > TRIE_RULE_SIZE ? HASH_RULE_SIZE : TRIE_RULE_SIZE)

struct aflow_rule {
    struct netdev_flow_key key;
    //FIXME: use RCU lock for actions
    struct dp_netdev_actions *actions;
    struct aflow_batch *batch;

    uint8_t rulebuf[RULE_MAX - RULE_COMMON_SIZE];
};

//Algorithmic flow table
struct aflow_table {
    uint8_t id;
    uint8_t type;
    int priority;

    union {
        struct aflow_hash hash_table;
    }af;
};

void aflow_hash_table_init(struct aflow_hash *h);
inline struct aflow_hash_rule * aflow_hash_table_find(struct aflow_hash *h, struct netdev_flow_key *key);
void aflow_hash_rule_init(struct aflow_hash_rule *rule);
void aflow_hash_rule_uinit(struct aflow_hash_rule *rule);
static void aflow_hash_table_uinit(struct aflow_hash *h);
void aflow_hash_rule_insert(struct aflow_hash *h,
        struct match *match, struct dp_netdev_actions *actions);

void aflow_hash_rule_delete(struct aflow_hash *h,
        struct match *match);


void aflow_init(void);
void aflow_uinit(void);
struct aflow_table* aflow_find_table(uint8_t table_id);

int aflow_dpctl_table_op(uint8_t type, uint8_t table_id, 
        const char *match_s, const char *actions_s, struct dpif *dpif);

int aflow_dpctl_add_table(uint8_t type, uint8_t id, int priority);
int aflow_dpctl_del_table(uint8_t id);

#endif
