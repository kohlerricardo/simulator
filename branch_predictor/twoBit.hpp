class twoBit_t{

    public:
        btb_t *btb;
        uint32_t btbHits;
		uint32_t btbMiss;
		uint32_t branchTaken;
		uint32_t branchNotTaken;
		uint32_t branches;
		uint32_t branchTakeMiss;
		uint32_t branchNotTakenMiss;
        // para acesso direto btb
        uint32_t index;
        uint8_t way;
    void allocate();
    void statistics();
    uint32_t predict(uint64_t address);
    void train();
    inline uint32_t searchLRU(btb_t *btb);
    uint32_t installLine(opcode_package_t instruction);
    uint32_t searchLine(uint64_t pc);
};