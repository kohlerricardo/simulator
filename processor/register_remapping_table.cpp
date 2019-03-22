#include "../simulator.hpp"
// =======================================================================
register_remapping_table_t::register_remapping_table_t(){
    this->package_clean();
}
 // =======================================================================
register_remapping_table_t::~register_remapping_table_t(){
    
}
 // =======================================================================
void register_remapping_table_t::package_clean(){
    this->register_core = POSITION_FAIL;
    this->entry = NULL;
}

// =======================================================================
void register_remapping_table_t::print_rrt_entry(){
    ORCS_PRINTF("Register_Core %d Entry %p\n",this->register_core,(void*)this->entry)
}
// =======================================================================
