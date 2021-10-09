
/*unsigned int INTF   ;
unsigned int INTC   ;*/
unsigned int MODE  = 0x00000000;
unsigned int STATUS= 0x0000c000;
unsigned int RXBUF = 0x00000000;

//unsigned int TXBUF   ;


    unsigned int RX_TRIG;   //trigger
    unsigned int TX_TRIG;

    //internal variables
    unsigned int divisor;
    unsigned int b_count;

    char tx_bf[4]={'a','b','c','d'};
    char rx_bf[4];
    unsigned int waddr;
    unsigned int raddr;
    unsigned int push_ptr = 4;
    unsigned int pop_ptr;
    unsigned int rcv_push_ptr;
    unsigned int rcv_pop_ptr;
    unsigned int rcv_waddr;
    unsigned int rcv_raddr;
    unsigned int TxREG;
    unsigned int RxREG;
    unsigned int TSR;
    unsigned int RSR;
    unsigned int count;
    unsigned int t_count;

    //unsigned int clk_cnt=0;

    //vcd
    //const char* vcd_fname;
    std::ofstream dump_fh;
    unsigned int dump_event;

    //states
    enum state_trns { IDLE_T, REG_LOAD_T, START_BIT_T, SHIFT_T, STOP_BIT_T };
    enum state_trns state_t = IDLE_T;
    //state_t = IDLE_T;

    enum state_rcv { IDLE_R, START_BIT_R, SHIFT_R, STOP_BIT_R, REG_LOAD_R };
    enum state_rcv state_r = IDLE_R;
    //state_r = IDLE_R;
    //internal functions

    void baud_gen();
    void trns_fifo_push();
    void trns_fifo_pop();
    void trns_fsm();
    void rcv_fifo_push();
    void rcv_fifo_pop();
    void rcv_fsm();

    void vcd_initialize(std::ofstream& dump_fh);
    void vcd_close();


    bool sck;               //baud_clock

    unsigned int TXD;       //uart
    unsigned int RXD = TXD;
    /*unsigned int RTS;
    unsigned int CTS;*/

    /*unsigned int PADDR;     //APB_slave
    unsigned int PSEL;
    unsigned int PENABLE;
    unsigned int PWRITE;
    unsigned int PWDATA;
    unsigned int PRDATA;
    unsigned int PREADY;
    unsigned int PSLVERR;*/

   /* uart(unsigned int base) : peripheral_class(base)
     {
         INTF  = 0x00000000;
         INTC  = 0x00000000;
         MODE  = 0x00000000;
         STATUS= 0x0000c000;
         RXBUF = 0x00000000;
         TXBUF = 0x00000000;
         state_t = IDLE_T;
         state_r = IDLE_R;
     };

     uart(unsigned int base, memory_obj* alloced_mem_obj) : peripheral_class(base,alloced_mem_obj)
     {
         INTF  = 0x00000000;
         INTC  = 0x00000000;
         MODE  = 0x00000000;
         STATUS= 0x0000c000;
         RXBUF = 0x00000000;
         TXBUF = 0x00000000;
         state_t = IDLE_T;
         state_r = IDLE_R;
     };

      uart(unsigned int base, memory_obj* alloced_mem_obj, std::ofstream& dump_fh) : peripheral_class(base,alloced_mem_obj)
      {
         INTF  = 0x00000000;
         INTC  = 0x00000000;
         MODE  = 0x00000000;
         STATUS= 0x0000c000;
         RXBUF = 0x00000000;
         TXBUF = 0x00000000;
         state_t = IDLE_T;
         state_r = IDLE_R;
         dump_event = 0;
         vcd_initialize(dump_fh);
      };
*/
       int eval_single_cycle(unsigned int clock);

       char* u2bin(unsigned int n);

       void vcd_dump(std::ofstream& dump_fh, unsigned int sim_time);

