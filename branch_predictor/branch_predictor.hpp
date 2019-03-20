class branch_predictor_t{

    public:
        //===================================   
        //atributos BTB
        //===================================
        btb_t *btb;
        uint32_t btbHits;
	uint32_t btbMiss;
        //===================================
        // para acesso direto btb
        //===================================
        uint32_t index;
        uint8_t way;
        //===================================
        //metodos para btb
        //===================================
        inline uint32_t searchLRU(btb_t *btb);
        uint32_t installLine(opcode_package_t instruction);
        uint32_t searchLine(uint64_t pc);
        //===================================
        //Atributos branch predictor
        //===================================
        uint32_t branchTaken;
        uint32_t branchNotTaken;
        uint32_t branches;
        uint32_t branchTakenMiss;
        uint32_t branchNotTakenMiss;
#if TWO_BIT
        twoBit_t *branchPredictor;
#else
        piecewise_t *branchPredictor;
#endif
        //===================================
        //metodos branch predictor
        //===================================
        branch_predictor_t();
        ~branch_predictor_t();
        void allocate();
        uint32_t solveBranch(opcode_package_t instruction, opcode_package_t nextOpcode);
        void statistics();
        void reset_statistics();

};