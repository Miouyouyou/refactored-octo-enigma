#include <src/nodes.h>
#include <myy/helpers/memory.h>

#include <myy/helpers/log.h>

unsigned int const nodes_parts_size =
	sizeof(node_container) + sizeof(node_content) + sizeof(node_related_data);
unsigned int const nodes_conts_size =
	sizeof(node_container) + sizeof(node_content);

uint8_t nodes_storage_create
(nodes * __restrict const nodes,
 unsigned int const n_nodes)
{
	uint8_t status = 0;

	unsigned int const allocated_size = nodes_parts_size * n_nodes;
	void * const allocated_space_addr =
		allocate_durable_memory(allocated_size);

	if (allocated_space_addr == NULL)
		goto could_not_allocate_storage;

	clean_memory_space(allocated_space_addr, allocated_size);

	nodes->containers.data = allocated_space_addr;
	nodes->contents.data =
		allocated_space_addr+(n_nodes*sizeof(node_container));
	nodes->associated_data =
		allocated_space_addr+(n_nodes*nodes_conts_size);
	nodes->count = 0;
	nodes->max = n_nodes;

	status = 1;

could_not_allocate_storage:
	return status;
}

uint8_t nodes_storage_reallocate
(nodes * __restrict const these_nodes, uint16_t const at_least)
{
	uint8_t status = 0;

	unsigned int const old_max = these_nodes->max;
	unsigned int const new_max = at_least * 2;

	unsigned int const new_size = new_max * nodes_parts_size;


	void * __restrict const old_start_addr =
		these_nodes->containers.data;
	void * __restrict const reallocated_space_addr =
		reallocate_durable_memory(old_start_addr, new_size);
	unsigned int const new_data_offset =
		(new_max * nodes_conts_size);
	unsigned int const node_container_size = sizeof(node_container);
	unsigned int const new_contents_offset =
		(new_max * node_container_size);

	if (reallocated_space_addr == NULL)
		goto could_not_reallocate_storage;

	nodes * __restrict const old_data_address =
		reallocated_space_addr + (old_max * nodes_conts_size);
	nodes * __restrict const new_data_address =
		reallocated_space_addr + new_data_offset;
	unsigned int const data_size =
		old_max * sizeof(node_related_data);

	recopy_inside_memory_space(
		new_data_address, old_data_address, data_size
	);

	node_content * __restrict const old_content_address =
		reallocated_space_addr + old_max * node_container_size;
	node_content * __restrict const new_content_address =
		reallocated_space_addr + new_contents_offset;

	unsigned int const node_content_size = sizeof(node_content);
	unsigned int const contents_size = old_max * node_content_size;

	recopy_inside_memory_space(
		new_content_address, old_content_address, contents_size
	);

	these_nodes->containers.data = reallocated_space_addr;
	these_nodes->contents.data =
		reallocated_space_addr+new_contents_offset;
	these_nodes->contents.data =
		reallocated_space_addr+new_data_offset;
	these_nodes->max = new_max;

	status = 1;

could_not_reallocate_storage:
	LOG("[nodes_storage_reallocate]\n  status : %u\n", status);
	return status;
}

void nodes_storage_delete_element
(nodes * __restrict const nodes,
 unsigned int const index)
{

	unsigned int const elements_to_copy =
		nodes->count - index;
	unsigned int const next_index =
		index+1;
	recopy_inside_memory_space(
		nodes->containers.data+index,
		nodes->containers.data+next_index,
		elements_to_copy * sizeof(node_container)
	);
	recopy_inside_memory_space(
		nodes->contents.data+index,
		nodes->contents.data+next_index,
		elements_to_copy * sizeof(node_content)
	);
	recopy_inside_memory_space(
		nodes->associated_data+index,
		nodes->associated_data+next_index,
		elements_to_copy * sizeof(node_related_data)
	);

	nodes->count--;
}

