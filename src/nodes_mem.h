#ifndef MYY_SRC_NODES_MEM_H
#define MYY_SRC_NODES_MEM_H 1

#include <src/nodes.h>

uint8_t nodes_storage_create
(nodes * __restrict const nodes,
 unsigned int const n_nodes);

uint8_t nodes_storage_reallocate
(nodes * __restrict const these_nodes,
 uint16_t const at_least);

void nodes_storage_delete_element
(nodes * __restrict const nodes,
 unsigned int const index);

#endif
