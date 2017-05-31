/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "core/sid_allocator.h"
#include "core/link_mgmt.h"
#include "core/topology.h"
#include "utilities/logging.h"
#include "utilities/memory.h"
#include "hal/interfaces.h"

#define LEADER_DEF_BITMAP 0x0ffe
#define ROUTER_DEF_BITMAP 0xfffe

void allocator_init(network_context_t *network)
{
    uint16_t sid;
    ssid_allocator_t *allocator;
    uint16_t index;

    allocator_deinit(network);
    allocator = (ssid_allocator_t *)ur_mem_alloc(sizeof(ssid_allocator_t));
    if (allocator == NULL) {
        return;
    }
    network->sid_base = (sid_base_t *)allocator;
    slist_init(&allocator->base.node_list);
    set_state_to_neighbor();

    allocator->base.node_num = 0;
    allocator->pf_node_num = 0;

    sid = mm_get_local_sid();
    if (mm_get_device_state() == DEVICE_STATE_SUPER_ROUTER) {
        sid = SUPER_ROUTER_SID;
    }

    if (sid == LEADER_SID) {
        allocator->attach_free_bits = LEADER_DEF_BITMAP;
    } else {
        allocator->attach_free_bits = ROUTER_DEF_BITMAP;
    }

    memset(allocator->mobile_free_bits, 0xff, sizeof(allocator->mobile_free_bits));
    allocator->mobile_free_bits[0] -= 1;

    for (index = 0; index < SID_LEN; index += SID_MASK_LEN) {
        uint16_t mask = (1 << SID_MASK_LEN) - 1;
        mask <<= index;
        if (sid & mask)
            break;
    }

    allocator->sid_shift = index - SID_MASK_LEN;
    allocator->sid_prefix = sid;
}

void allocator_deinit(network_context_t *network)
{
    ssid_allocator_t *allocator;
    sid_node_t *node;

    allocator = (ssid_allocator_t *)network->sid_base;
    if (allocator == NULL) {
        return;
    }
    while (!slist_empty(&allocator->base.node_list)) {
        node = slist_first_entry(&allocator->base.node_list, sid_node_t, next);
        slist_del(&node->next, &allocator->base.node_list);
        ur_mem_free(node, sizeof(sid_node_t));
    }
    allocator->base.node_num = 0;
    ur_mem_free(allocator, sizeof(ssid_allocator_t));
    network->sid_base = NULL;
}

bool is_direct_child(network_context_t *network, uint16_t sid)
{
    ssid_allocator_t *allocator;
    uint16_t sidmask = 0;
    uint8_t  index;

    if (network == NULL || sid == LEADER_SID || sid == INVALID_SID) {
        return false;
    }
    allocator = (ssid_allocator_t *)network->sid_base;
    if (allocator == NULL) {
        return false;
    }

    index = SID_LEN - SID_MASK_LEN;
    while (index > allocator->sid_shift) {
        sidmask |= (SID_MASK << index);
        index -= SID_MASK_LEN;
    }
    if (allocator->sid_prefix != (sid & sidmask ) ||
        allocator->sid_prefix == sid) {
        return false;
    }
    sidmask = (1 << allocator->sid_shift) - 1;
    if (sid & sidmask) {
        return false;
    }

    return true;
}

ur_error_t update_sid_mapping(network_context_t *network,
                              ur_node_id_t *node_id, bool to_add)
{
    ssid_allocator_t *allocator;
    sid_node_t *node;
    sid_node_t *new_node = NULL;

    allocator = (ssid_allocator_t *)network->sid_base;
    slist_for_each_entry(&allocator->base.node_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, node_id->ueid, sizeof(node->node_id.ueid)) == 0) {
            new_node = node;
            break;
        }
    }

    if (to_add == false) {
        if (node) {
            free_sid(network, node_id->sid);
            slist_del(&node->next, &allocator->base.node_list);
        }
        return UR_ERROR_NONE;
    }

    if (new_node) {
        if (node_id->sid != INVALID_SID && node_id->sid != new_node->node_id.sid) {
            free_sid(network, new_node->node_id.sid);
        }
    } else {
        new_node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (!new_node)
            return UR_ERROR_MEM;
        slist_add(&new_node->next, &allocator->base.node_list);
    }

    memcpy(new_node->node_id.ueid, node_id->ueid, sizeof(new_node->node_id.ueid));
    new_node->node_id.sid = node_id->sid;
    new_node->node_id.attach_sid = node_id->attach_sid;

    return UR_ERROR_NONE;
}

void free_sid(network_context_t *network, uint16_t sid)
{
    ssid_allocator_t *allocator;
    uint8_t len;

    allocator = (ssid_allocator_t *)network->sid_base;
    if (is_partial_function_sid(sid)) {
        uint16_t idx = sid - (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        if (release_bit(allocator->mobile_free_bits, PF_NODE_NUM, idx)) {
            allocator->pf_node_num --;
        }
        return;
    }

    if (!is_direct_child(network, sid)) {
        return;
    }
    sid -= allocator->sid_prefix;
    sid >>= allocator->sid_shift;
    len = 1 << SID_MASK_LEN;
    if (allocator->sid_prefix == LEADER_SID) {
        len = 12;
    }
    if (release_bit(&allocator->attach_free_bits, len, sid)) {
        allocator->base.node_num --;
    }
}

static ur_error_t allocate_expected_sid(network_context_t *network,
                                        ur_node_id_t *node_id)
{
    ssid_allocator_t *allocator;
    uint8_t          index;
    uint8_t          len;

    allocator = (ssid_allocator_t *)network->sid_base;
    if (is_direct_child(network, node_id->sid)) {
        index = (node_id->sid - allocator->sid_prefix) >> allocator->sid_shift;
        len = 16;
        if (allocator->sid_prefix == LEADER_SID) {
            len = 12;
        }
        if ((index > len) ||
            (grab_free_bit(&allocator->attach_free_bits, len, index) == UR_ERROR_FAIL)) {
            return UR_ERROR_FAIL;
        }
        allocator->base.node_num++;
    } else if (is_partial_function_sid(node_id->sid)) {
        index = node_id->sid - (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        if (index > PF_NODE_NUM) {
            return UR_ERROR_FAIL;
        }
        if (grab_free_bit(allocator->mobile_free_bits, PF_NODE_NUM, index) == UR_ERROR_FAIL) {
            return UR_ERROR_FAIL;
        }
        if (update_sid_mapping(network, node_id, true) != UR_ERROR_NONE) {
            release_bit(allocator->mobile_free_bits, PF_NODE_NUM, node_id->sid);
            return UR_ERROR_FAIL;
        }
        allocator->pf_node_num++;
    } else {
        return UR_ERROR_FAIL;
    }
    return UR_ERROR_NONE;
}

ur_error_t allocate_sid(network_context_t *network, ur_node_id_t *node_id)
{
    ssid_allocator_t *allocator;
    neighbor_t       *node = NULL;
    sid_node_t       *sid_node = NULL;
    int               newsid = -1;

    allocator = (ssid_allocator_t *)network->sid_base;
    node = get_neighbor_by_ueid(node_id->ueid);
    if (node == NULL) {
        goto new_sid;
    }

    if (node_id->sid != INVALID_SID &&
        allocate_expected_sid(network, node_id) == UR_ERROR_FAIL) {
        goto new_sid;
    }

    if (node_id->sid == INVALID_SID) {
        goto new_sid;
    }

    node_id->type = allocator->sid_shift == 0 ? LEAF_NODE : ROUTER_NODE;
    return UR_ERROR_NONE;

new_sid:
    if (node_id->mode & MODE_MOBILE) {
        slist_for_each_entry(&allocator->base.node_list, sid_node, sid_node_t, next) {
            if (memcmp(sid_node->node_id.ueid, node_id->ueid,
                       sizeof(sid_node->node_id.ueid)) == 0) {
                break;
            }
        }
        node_id->type = LEAF_NODE;
        if (sid_node == NULL) {
            newsid = find_first_free_bit(allocator->mobile_free_bits, PF_NODE_NUM);
            node_id->sid = ((uint16_t)newsid) | (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        } else {
            newsid = 0;
            node_id->sid = sid_node->node_id.sid;
        }
    } else {
        if (allocator->sid_prefix == LEADER_SID) {
            newsid = find_first_free_bit(&allocator->attach_free_bits, 12);
        } else {
            newsid = find_first_free_bit(&allocator->attach_free_bits, 16);
        }
        node_id->sid = allocator->sid_prefix | (((uint16_t)newsid) << allocator->sid_shift);
        node_id->type = allocator->sid_shift == 0 ? LEAF_NODE : ROUTER_NODE;
    }

    if (newsid < 0) {
        return UR_ERROR_MEM;
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
            "allocate 0x%04x\r\n", node_id->sid);

    if (!(node_id->mode & MODE_MOBILE)) {
        allocator->base.node_num++;
        goto out;
    }

    if (UR_ERROR_NONE != update_sid_mapping(network, node_id, true)) {
        release_bit(allocator->mobile_free_bits, PF_NODE_NUM, newsid);
        return UR_ERROR_FAIL;
    }
    if (newsid > 0) {
        allocator->pf_node_num++;
    }

out:
    return UR_ERROR_NONE;
}

uint16_t get_allocated_number(network_context_t *network)
{
    ssid_allocator_t *allocator;

    allocator = (ssid_allocator_t *)network->sid_base;
    return allocator->base.node_num;
}

uint32_t get_allocated_bitmap(network_context_t *network)
{
    ssid_allocator_t *allocator;

    allocator = (ssid_allocator_t *)network->sid_base;
    if (allocator->sid_prefix == LEADER_SID)
        return LEADER_DEF_BITMAP & ~allocator->attach_free_bits;
    return ROUTER_DEF_BITMAP & ~allocator->attach_free_bits;
}


uint16_t get_allocated_pf_number(network_context_t *network)
{
    ssid_allocator_t *allocator;

    allocator = (ssid_allocator_t *)network->sid_base;
    return allocator->pf_node_num;
}

uint16_t get_free_number(network_context_t *network)
{
    ssid_allocator_t *allocator;

   allocator = (ssid_allocator_t *)network->sid_base;
    if (allocator->sid_prefix == LEADER_SID)
        return 11 - allocator->base.node_num;
    return 15 - allocator->base.node_num;
}

slist_t *get_ssid_nodes_list(network_context_t *network)
{
    ssid_allocator_t *allocator;

    if (network == NULL) {
        return NULL;
    }
    allocator = (ssid_allocator_t *)network->sid_base;
    return &allocator->base.node_list;
}

bool is_partial_function_sid(uint16_t sid)
{
    if (sid == INVALID_SID || sid == BCAST_SID) {
        return false;
    }
    if (((sid >> PF_SID_PREFIX_OFFSET) & PF_SID_PREFIX_MASK) >= MOBILE_PREFIX) {
        return true;
    }
    return false;
}
