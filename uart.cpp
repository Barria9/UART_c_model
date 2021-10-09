#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
//#include "list_mem.h"
//#include "utils.h"
//#include "peripheral_class.hpp"
#include "uart.hpp"


int main()
{
	int clock = 0;
	int half_cycle_cnt = 0;

  const char *vcd_fname = "dump.vcd";
  std::ofstream dump_fh;
  dump_fh.open(vcd_fname);
  if(!dump_fh)
    std::cerr<<"Can not open Dump file ("<<vcd_fname<<") for SoC"<<std::endl;

    MODE = 0x0010088;

	for(i=0;i<1000;i++)
	{

		half_cycle_cnt = half_cycle_cnt + 5;
		clock = 1;
		eval_single_cycle(clock);
		trns_fsm();
		dump_event++;

		half_cycle_cnt = half_cycle_cnt + 5;
		clock = 0;

		dump_event++;

	}

}


int eval_single_cycle(unsigned int clock)
{
   /* if(reset_n)
    {
    	 INTF  = 0x00000000;
         INTC  = 0x00000000;
         MODE  = 0x00000000;
         STATUS= 0x0000c000;
         RXBUF = 0x00000000;
         TXBUF = 0x00000000;
    }*/

    dump_event = 0;


    if(clock == 1)
    {
    	//printf("eval_single_cycle\n");


           if((MODE & 0x08)==0x08)
           { // MODE = 0x00010088
            divisor = 4*(((MODE & 0xFFFF0000)>>16) +1);
            //printf("high_speed_baud\n divisor = %d\n", divisor);
           }
           else
           {
            divisor = 16*(((MODE & 0xFFFF0000)>>16) +1);
            //printf("low_speed_baud\n");
           }


           dump_event++;


        //clk_cnt++;
        //printf("number of passed clock = %d\n",clk_cnt );

       //printf("before baud_gen\n");
       baud_gen();
       //printf("after baud_gen\n");
       //trns_fifo_pop();
       //trns_fsm();
       rcv_fsm();
      // rcv_fifo_pop();

    }
}

void trns_fsm()
{
  switch(state_t)
  {
    case IDLE_T:
      TXD = 1;
      t_count = 0;
      //STATUS = STATUS | 0x40;

      if(((MODE & 0x80)==0x80)&& ((STATUS & 0x8000)== 0x0000))
        {
          state_t = REG_LOAD_T;

        }
        else if ((STATUS & 0x8000)== 0x8000)
        {
        	STATUS = STATUS | 0x40;
        }

        dump_event++;
        break;

    case REG_LOAD_T:

         state_t = START_BIT_T;
          trns_fifo_pop();


       dump_event++;
       break;

    case START_BIT_T:

      TSR = TxREG;


      if(sck==1)
      {
        state_t = SHIFT_T;
        TXD = 0;
      }
      dump_event++;
      break;

    case SHIFT_T:

      if(sck==1)
      {
        TXD = TSR & 0X01;
        TSR = TSR >> 1;
        t_count++;
      }

      if(t_count>=8)
      {
        state_t = STOP_BIT_T;
      }
      dump_event++;
      break;

    case STOP_BIT_T:
      t_count = 0;

      if(sck==1)
      {
        state_t = IDLE_T;
        TXD = 1;
      }
      //STATUS = STATUS | 0x40;
      dump_event++;
      break;

    default:
        state_t = IDLE_T;
        dump_event++;
        break;
  }
}

void rcv_fsm()
{
  switch(state_r)
  {
    case IDLE_R:

      if((MODE & 0x08) ==0x08)
      {
        state_r = IDLE_R;
      }

    case START_BIT_R:

      if(RXD==0 && sck==1)
      {
        state_r = SHIFT_R;
      }
      count = 0;
      dump_event++;
      break;

    case SHIFT_R:
     if(sck)
      {
        RSR = RSR | (RXD << count);
      }

      if(count<=7)
      {
        state_r = STOP_BIT_R;
      }
     count++;
     dump_event++;
     break;

    case STOP_BIT_R:
      if(RXD==0 && sck==1)
      {
        state_r = IDLE_R;
      }
      rcv_fifo_push();
      count = 0;
      dump_event++;
      break;

    default:
      state_r = IDLE_R;
      dump_event++;
      break;
  }
}

/*void trns_fifo_push()
{
  if((STATUS & 0x2020)==0x0000) //need to confirm
  {

    waddr = push_ptr%4;
    tx_bf [waddr] = TXBUF;
    push_ptr++;
    STATUS = (STATUS & 0x00FF7FBF)|((push_ptr-pop_ptr)<<24) ; //STATUS = STATUS & 0xFFFFFFBF;
  }

  if((push_ptr-pop_ptr)==4)
  {
    STATUS = STATUS |0x2020 ;
  }
}*/

void trns_fifo_pop()
{
  if((STATUS & 0x8000)== 0x00)
  {
    raddr = pop_ptr%4;
    TxREG = tx_bf[raddr];
   // tx_bf[raddr] = 0;
    pop_ptr++;
    STATUS = (STATUS & 0x00FFDFEF)|((push_ptr-pop_ptr)<<24) ;
  }

  if(push_ptr==pop_ptr)
  {
    STATUS = STATUS | 0x8000;
  }
}

void  rcv_fifo_push()
{
  if(!RXBUF && (STATUS & 0x1000)!= 0x1000)
   {
     rcv_waddr = rcv_push_ptr%4;
     rx_bf[rcv_waddr] = RxREG;
     rcv_push_ptr++;
     STATUS = STATUS | 0x01;
     STATUS = (STATUS & 0xFF00BFFF)|((rcv_push_ptr-rcv_pop_ptr)<<16)  ;
   }

   if((rcv_push_ptr-rcv_pop_ptr)==4)
   {
     STATUS = STATUS | 0x1000;
   }
}

void  rcv_fifo_pop()
{
  if((STATUS & 0x4001)==0x01 )
  {
    rcv_raddr = rcv_pop_ptr%4;
    RXBUF = rx_bf[rcv_raddr];
    rx_bf[rcv_raddr] = 0;
    rcv_pop_ptr++;
    STATUS = (STATUS & 0xFF00EFFF)|((rcv_push_ptr-rcv_pop_ptr)<<16);
  }

  if(rcv_push_ptr==rcv_pop_ptr)
  {
    STATUS = STATUS | 0x4000;
  }
}

void baud_gen()
{
    if((MODE & 0x80)== 0x00)
    {
        b_count = 0;
        sck = 0;
        dump_event++;
        //printf("baud_genarator_disable\n");
    }
    else
    {
    	 //printf("baud_genarator\n");
         b_count++;

         //printf("baud_genarator");

         if((b_count%divisor)==0)
         {
             sck = 1;
             dump_event++;
         }
         else
         {
             sck = 0;
             dump_event++;
         }

    }
}

char* u2bin(unsigned int n)
{
  int c, k, count = 0;
  char* bin = (char*)calloc(32,sizeof(char));

  for (c = 31; c >= 0; c--)
  {
    k = n >> c;
    if (k & 1)
      *(bin+count) = '1';
    else
      *(bin+count) = '0';
    count++;
  }
  return bin;
}



void vcd_initialize(std::ofstream& dump_fh)
{
    dump_fh<<"$version Generated by ARSIM++ v0.1 $end"<<std::endl;
    dump_fh<<"$timescale 1ns $end"<<std::endl;
    dump_fh<<"$scope\n\t module uart\n$end"<<std::endl;

    //dump_fh<<"$var wire 32 u_TF INTF   $end"<<std::endl;
    //dump_fh<<"$var wire 32 u_TC INTC   $end"<<std::endl;
    dump_fh<<"$var wire 32 u_MD MODE   $end"<<std::endl;
    dump_fh<<"$var wire 32 u_ST STATUS $end"<<std::endl;
    dump_fh<<"$var wire 32 u_RB RXBUF  $end"<<std::endl;
    //dump_fh<<"$var wire 32 u_TB TXBUF  $end"<<std::endl;
    dump_fh<<"$var wire  1 u_k  sck    $end"<<std::endl;
    dump_fh<<"$var wire  1 u_Tx TXD $end"<<std::endl;
    dump_fh<<"$var wire  1 u_Rx RXD $end"<<std::endl;
    dump_fh<<"$var wire 32 u_TS TSR $end"<<std::endl;
    //dump_fh<<"$var wire 32 u_RT RTS $end"<<std::endl;
    //dump_fh<<"$var wire 32 u_CT CTS $end"<<std::endl;
    /*dump_fh<<"$var wire 32 u_PA PADDR    $end"<<std::endl;
    dump_fh<<"$var wire  1 u_PS PSEL     $end"<<std::endl;
    dump_fh<<"$var wire  1 u_PN PENABLE  $end"<<std::endl;
    dump_fh<<"$var wire  1 u_PW PWRITE   $end"<<std::endl;
    dump_fh<<"$var wire 32 u_PD PWDATA   $end"<<std::endl;
    dump_fh<<"$var wire 32 u_PR PRDATA   $end"<<std::endl;
    dump_fh<<"$var wire  1 u_RD PREADY   $end"<<std::endl;
    dump_fh<<"$var wire  1 u_SE PSLVERR  $end"<<std::endl; */
    dump_fh<<"$var wire 32 u_st state_t $end"<<std::endl;
    dump_fh<<"$var wire 32 u_sr state_r $end"<<std::endl;
    dump_fh<<"$var wire 32 u_c  count $end"<<std::endl;
    dump_fh<<"$var wire 32 u_bc b_count $end"<<std::endl;
    dump_fh<<"$var wire 32 u_psh push_ptr $end"<<std::endl;
    dump_fh<<"$var wire 32 u_pp  pop_ptr $end"<<std::endl;
    dump_fh<<"$var wire 32 u_rpsh rcv_push_ptr $end"<<std::endl;
    dump_fh<<"$var wire 32 u_rpp  rcv_pop_ptr $end"<<std::endl;
    //dump_fh<<"$upscope $end"<<std::endl;
    dump_fh<<"$enddefinitions $end"<<std::endl;
    dump_fh<<"#0"<<std::endl;
    dump_fh<<"$dumpvars"<<std::endl;
   /* dump_fh<<"b0 u_TF"<<std::endl;
    dump_fh<<"b0 u_TC"<<std::endl;*/
    dump_fh<<"b0 u_MD"<<std::endl;
    dump_fh<<"b0 u_ST"<<std::endl;
    dump_fh<<"b0 u_RB"<<std::endl;
    dump_fh<<"b0 u_TB"<<std::endl;
    dump_fh<<"b0 u_k"<<std::endl;
    dump_fh<<"b0 u_Tx"<<std::endl;
    dump_fh<<"b0 u_Rx"<<std::endl;
    dump_fh<<"b0 u_TS"<<std::endl;
    //dump_fh<<"b0 u_RT"<<std::endl;
    //dump_fh<<"b0 u_CT"<<std::endl;
    /*dump_fh<<"b0 u_PA"<<std::endl;
    dump_fh<<"b0 u_PS"<<std::endl;
    dump_fh<<"b0 u_PN"<<std::endl;
    dump_fh<<"b0 u_PW"<<std::endl;
    dump_fh<<"b0 u_PD"<<std::endl;
    dump_fh<<"b0 u_PR"<<std::endl;
    dump_fh<<"b0 u_RD"<<std::endl;
    dump_fh<<"b0 u_SE"<<std::endl;*/
    dump_fh<<"b0 u_c"<<std::endl;
    dump_fh<<"b0 u_bc"<<std::endl;
    dump_fh<<"b0 u_psh"<<std::endl;
    dump_fh<<"b0 u_pp"<<std::endl;
    dump_fh<<"b0 u_rpsh"<<std::endl;
    dump_fh<<"b0 u_rpp"<<std::endl;
    dump_fh<<"$end"<<std::endl;
}

void uart::vcd_dump(std::ofstream& dump_fh, unsigned int sim_time)
{
    char* bin;
    if(dump_event>0)
    {
        dump_fh<<"#"<<sim_time<<std::endl;
        bin = u2bin(INTF); dump_fh<<"b"<<bin<<"   u_TF"<<std::endl;
        bin = u2bin(INTC); dump_fh<<"b"<<bin<<"   u_TC"<<std::endl;
        bin = u2bin(MODE); dump_fh<<"b"<<bin<<"   u_MD"<<std::endl;
        bin = u2bin(STATUS); dump_fh<<"b"<<bin<<" u_ST"<<std::endl;
        bin = u2bin(RXBUF); dump_fh<<"b"<<bin<<"  u_RB"<<std::endl;
        bin = u2bin(TXBUF); dump_fh<<"b"<<bin<<"  u_TB"<<std::endl;
        bin = u2bin(sck); dump_fh<<"b"<<bin<<" u_k"<<std::endl;
        bin = u2bin(TXD); dump_fh<<"b"<<bin<<" u_Tx"<<std::endl;
        bin = u2bin(RXD); dump_fh<<"b"<<bin<<" u_Rx"<<std::endl;
        bin = u2bin(TSR); dump_fh<<"b"<<bin<<" u_TS"<<std::endl;
        /*bin = u2bin(RTS); dump_fh<<"b"<<bin<<" u_RT"<<std::endl;
        bin = u2bin(CTS); dump_fh<<"b"<<bin<<" u_CT"<<std::endl;*/
        /*bin = u2bin(PADDR); dump_fh<<"b"<<bin<<"   u_PA"<<std::endl;
        bin = u2bin(PSEL); dump_fh<<"b"<<bin<<"    u_PS"<<std::endl;
        bin = u2bin(PENABLE); dump_fh<<"b"<<bin<<" u_PN"<<std::endl;
        bin = u2bin(PWRITE); dump_fh<<"b"<<bin<<"  u_PW"<<std::endl;
        bin = u2bin(PWDATA); dump_fh<<"b"<<bin<<"  u_PD"<<std::endl;
        bin = u2bin(PRDATA); dump_fh<<"b"<<bin<<"  u_PR"<<std::endl;
        bin = u2bin(PREADY); dump_fh<<"b"<<bin<<"  u_RD"<<std::endl;
        bin = u2bin(PSLVERR); dump_fh<<"b"<<bin<<" u_SE"<<std::endl;*/
        bin = u2bin(state_t); dump_fh<<"b"<<bin<<" u_st"<<std::endl;
        bin = u2bin(state_r); dump_fh<<"b"<<bin<<" u_sr"<<std::endl;
        bin = u2bin(count); dump_fh<<"b"<<bin<<" u_c"<<std::endl;
        bin = u2bin(b_count); dump_fh<<"b"<<bin<<" u_bc"<<std::endl;
        bin = u2bin(push_ptr); dump_fh<<"b"<<bin<<" u_psh"<<std::endl;
        bin = u2bin(pop_ptr); dump_fh<<"b"<<bin<<" u_pp"<<std::endl;
        bin = u2bin(rcv_push_ptr); dump_fh<<"b"<<bin<<" u_psh"<<std::endl;
        bin = u2bin(rcv_pop_ptr); dump_fh<<"b"<<bin<<" u_pp"<<std::endl;
    }
}
