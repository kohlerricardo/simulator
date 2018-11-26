
class disambiguation_perfect_t
{
private:

public:
    disambiguation_perfect_t();
    ~disambiguation_perfect_t();
    void make_memory_dependencies(memory_order_buffer_line_t *mob_line);
    void solve_memory_dependencies(memory_order_buffer_line_t *mob_line);
    void statistics();
    void allocate();
     // Statistics
    uint64_t stat_disambiguation_read_false_positive;
    uint64_t stat_disambiguation_write_false_positive;
    uint64_t stat_address_to_address;


    INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_read_false_positive)
    INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_write_false_positive)
    INSTANTIATE_GET_SET_ADD(uint64_t,stat_address_to_address)
};


