#include "../../simulator.hpp"
#include <string>
emc_opcode_package_t::emc_opcode_package_t(){
    this->package_clean();
    this->reg_deps_ptr_array = NULL;
};
emc_opcode_package_t::~emc_opcode_package_t(){
utils_t::template_delete_array<emc_opcode_package_t*>(reg_deps_ptr_array);
};
void emc_opcode_package_t::package_clean(){
        this->uop.package_clean();
        this->mob_ptr = NULL;
        this->wait_reg_deps_number=0;
        this->wake_up_elements_counter=0;
        this->rob_ptr = NULL; 
};
std::string emc_opcode_package_t::content_to_string(){
    std::string content_string;
    content_string = "";
    
    content_string = content_string + this->uop.content_to_string();
    content_string = content_string + " | EMC register WAIT: " + utils_t::uint32_to_string(this->wait_reg_deps_number);
    content_string = content_string + " | Wake Up operations " + utils_t::uint32_to_string(this->wake_up_elements_counter);
    if(this->mob_ptr != NULL){
        content_string = content_string + this->mob_ptr->content_to_string();
    }
  
    return content_string;
};