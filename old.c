struct loader_state {
    int relocation_base;
    int is_sicxe;  // 0 == SIC, 1 == SICXE
    int program_start_addr;
};
void loader_initialization(int relocate_addr, int format){
    loader_state.relocation_base = relocate_addr; //relocation address provided by the user
    state.is_sicxe= format; //relocation format like SIC or SICXE
}
void header(char *line, FILE file){
    //this function processes the first header line
    if (fgets(line, sizeof(line), file){
        line[0]= 
    }
}
