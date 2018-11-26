#ifndef PRIORITY_MEMORY_ACCESS_H
#define PRIORITY_MEMORY_ACCESS_H
class priority_memory_access_t{
    public:
	bool operator()(memory_order_buffer_line_t *left, memory_order_buffer_line_t *right){
		return left->opcode_address > right->opcode_address;
	};
};
#endif 