
modbus_mapping_t* modbus_default_mapping_new(int nb_bits, int nb_input_bits,
                                     int nb_registers, int nb_input_registers);

void modbus_default_mapping_free(void* addr);
