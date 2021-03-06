/**
 * @file MB1_serial_com.h
 * @author  Bui Van Hieu <bvhieu@cse.hcmut.edu.vn>
 * @version 1.0
 * @date 07-07-2012
 *
 * @copyright
 * This poject and all of its relevant documents, source codes, compiled libraries are belong
 * to <b> Smart Sensing and Intelligent Controlling Group (SSAIC)</b> \n
 * You are prohibited to broadcast, distribute, copy, modify, print, or reproduce this in anyway
 * without written permission from SSAIC \n
 * <b> Copyright (C) 2012 by SSAIC, All Right Reversed </b>
 */

#ifndef __MB1_SERIAL_T_H
#define __MB1_SERIAL_T_H

#include "MB1_Glb.h"
#include "unistd.h"

/* stdStream */
#define USART_stdStream_stdout 0x1
#define USART_stdStream_stdin 0x2
#define USART_stdStream_stderr 0x4

namespace serial_ns {
enum flags_e:uint16_t {
    txe = USART_FLAG_TXE,
    rxne = USART_FLAG_RXNE,
};

enum it_flags: uint16_t {
    it_rxne = USART_IT_RXNE,
    it_txe = USART_IT_TXE,
    it_pe = USART_IT_PE,
    it_tc = USART_IT_TC,
    it_idle = USART_IT_IDLE,
    it_lbd = USART_IT_LBD,
    it_cts = USART_IT_CTS,
    it_err = USART_IT_ERR,
    it_ore = USART_IT_ORE,
    it_ne = USART_IT_NE,
    it_fe = USART_IT_FE,
};

}

class serial_t {
private:
  uint8_t usedUart;
public: serial_t(uint8_t usedUart);
  void  Restart(uint32_t baudRate);
  void  Shutdown(void);
  void  Print(uint8_t outChar);
  void  Print(char outChar);
  void  Print(uint8_t* outStr);
  void  Print(const char* outStr);
  void  Print(uint32_t outNum);
  void  Print(int32_t outNum);
  void  Out(uint8_t outNum);
  void  Out(uint16_t outNum);
  void  Out(uint32_t outNum);
  void  Out(uint8_t outBuf[], uint32_t bufLen);
  bool  Check_flag(uint16_t flag);
  void  Clear_flag(uint16_t flag);
  uint16_t Get (void);
  uint16_t Get_ISR (void);
  uint8_t Get_usedUart(void) {return usedUart + 1;};

  //enable retarget for printf.
  void Retarget (uint8_t stdStream);

  /* USART Interrupts */
  void it_enable(uint8_t prem_prio, uint8_t sub_prio);
  void it_disable(void);
  void it_config(uint16_t it_flags, bool enable);
  bool it_get_status(uint16_t it_flag);
};

//retarget functions to overload functions in stdio.h.
#ifdef __cplusplus
extern "C"{
#endif

int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void _ttywrch(int ch);

#ifdef __cplusplus
}
#endif

#endif /*__SERIAL_T_H */
