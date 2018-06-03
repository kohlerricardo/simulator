// =======================================================================
class register_remapping_table_t{
    public:
    int32_t register_core;
    emc_opcode_package_t *entry;
    void package_clean();
};
void register_remapping_table_t::package_clean(){
    this->register_core = POSITION_FAIL;
    this->entry = NULL;
};
 // =======================================================================