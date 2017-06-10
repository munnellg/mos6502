/*                                                                              
 * ============================================================================ 
 *                                                                              
 *       Filename:  gem_mos.h                                                      
 *                                                                              
 *    Description:                                                              
 *                                                                              
 *        Version:  1.0                                                         
 *        Created:  14/05/17 01:19:01                                           
 *       Revision:  none                                                        
 *       Compiler:  gcc                                                         
 *                                                                              
 *         Author:  Gary Munnelly (gm), munnellg@tcd.ie                         
 *        Company:  Adapt Centre, Trinity College Dublin                        
 *                                                                              
 * ============================================================================ 
 */  

#ifndef _GEM_MOS_H_
#define _GEM_MOS_H_

#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/*---------------------------------------------------------------------------
 *
 *  Status register control bits. Can be ORed with the status register to set
 *  a flag or inverted then ANDed to unset a bit
 *
 *---------------------------------------------------------------------------*/

#define SR_FLAG_NEGATIVE    0x80 /* negative flag bit in status register */
#define SR_FLAG_OVERFLOW    0x40 /* overflow flag bit in status register */
#define SR_FLAG_UNUSED      0x20 /* Unused, but still occasionally altered */
#define SR_FLAG_BREAK       0x10 /* break flag bit in status register */
#define SR_FLAG_DECIMAL     0x08 /* decimal flag bit in status register */
#define SR_FLAG_INTERRUPT   0x04 /* interrupt flag bit in status register */
#define SR_FLAG_ZERO        0x02 /* zero flag bit in status register */
#define SR_FLAG_CARRY       0x01 /* carry flag bit in status register */

#define SR_UPDATE           0x01
#define SR_NO_UPDATE        0x00

/*---------------------------------------------------------------------------
 *
 *  Memory map of the MOS 6502. Defines the regions in memory and where they
 *  are located.
 *
 *---------------------------------------------------------------------------*/

#define GEM_MEMORY_STACK    0x100   /* top of the stack */
#define GEM_ROM_ADDR        0x400   /* where user programs are loaded */
#define GEM_VECTOR_RESET    0xFFFC
#define GEM_VECTOR_NMI      0xFFFA
#define GEM_VECTOR_BRK      0xFFFE
#define GEM_VECTOR_IRQ      0xFFFE
#define GEM_MEMORY_CAPACITY 0x10000 /* the total capacity of MOS memory */

/*---------------------------------------------------------------------------
 *
 *  Test macros. Useful for checking if a particular bit in the status 
 *  register should be updated after an operation
 *
 *---------------------------------------------------------------------------*/

/* get most significant bit of value. */
#define GEM_TEST_MSB(x) \
            ( (int)((unsigned int)((int)(x))) >> (sizeof(x) * CHAR_BIT - 1))
/* get least significant bit of a value */
#define GEM_TEST_LSB(x) ( ((x) & 0x01) ) 

#define GEM_TEST_NEGATIVE(x) GEM_TEST_MSB(x) /* check msb to see if -ive */
#define GEM_TEST_ZERO(x) ((x) == 0 )
#define GEM_TEST_OVERFLOW( x, y, z ) \
    ((GEM_TEST_NEGATIVE(x) && GEM_TEST_NEGATIVE(y) && !GEM_TEST_NEGATIVE(z))||\
    (!GEM_TEST_NEGATIVE(x) && !GEM_TEST_NEGATIVE(y) && GEM_TEST_NEGATIVE(z)))
#define GEM_TEST_CARRY( x, y, m ) ( (m) - (x) < (y) )

#define GEM_REGISTER8_MAX ((1 << sizeof(GEM_REGISTER8) * CHAR_BIT) - 1)

#define GEM_GET_HBYTE(x) (((x)>>8)&0xFF)
#define GEM_GET_LBYTE(x) ((x)&0xFF)

/*---------------------------------------------------------------------------
 *
 *  Some handy typedefs, just in case we ever want to change how the emulator 
 *  stores information
 *
 *---------------------------------------------------------------------------*/

typedef int      GEM_TEST;          /* a boolean for test pass/fail */
typedef uint16_t GEM_MEMORY_ADDR;   /* address of a location in memory */
typedef uint16_t GEM_MEMORY_WORD;   /* 16 bit value in memory */
typedef uint8_t  GEM_MEMORY_BYTE;   /* 8 bit value in memory */
typedef int8_t   GEM_MEMORY_SBYTE;  /* signed 8 bit value in memory */
typedef int32_t  GEM_CLOCK_TICKS;   /* processor clock type */
typedef uint8_t  GEM_OPCODE;        /* mos instruction opcode */
typedef uint8_t  GEM_STATUS_FLAG;   /* status register flag bit */
typedef uint8_t  GEM_REGISTER8;     /* 8-bit register type  */
typedef uint16_t GEM_REGISTER16;    /* 16-bit register type */
typedef int8_t   GEM_OFFSET;

typedef struct gem_mos gem_mos;     /* forward declaration of the mos struct */

/* This should be the structure of an instruction executed by the emulator.
 * the instruction should take the emulator state as input, perform an 
 * operation and return the number of cycles which elapsed while the 
 * instruction was being run */
typedef GEM_CLOCK_TICKS (*gem_instruction)( gem_mos *m );

/*
 * ===  STRUCT  ===============================================================
 *          Name: gem_mos
 *  Desecription: A struct for storing the current state of the MOS 6502
 *
 *                The processor had 3 registers labelled A (or Accumulator), X 
 *                and Y for performing arithmetic operations. The vast majority 
 *                of operations are performed on the Accumulator with X and Y
 *                being used for storing intermediate values or dynamically 
 *                addressing memory.
 *
 *                The processor also features an 8 bit status register which
 *                tracks the outcome of certain operations as well as 
 *                controlling some of the behaviour of the processor. In order
 *                the flags are:
 *                  N   ....    Negative
 *                  V   ....    Overflow
 *                  -   ....    ignored
 *                  B   ....    Break
 *                  D   ....    Decimal (use BCD for arithmetics)
 *                  I   ....    Interrupt (IRQ disable)
 *                  Z   ....    Zero
 *                  C   ....    Carry
 *
 *                The stack pointer is capable of addressing 256 bytes of 
 *                memory. The top of the stack is typically located at
 *                0x100. In this emulator, the position of the top of the stack
 *                is stored in the GEM_MEMORY_STACK macro
 *
 *                The program counter keeps track of the current instruction
 *                being executed. It is also used to find the address of any
 *                arguments being passed to the instruction being executed. The
 *                value of the program counter may increase or decrease as
 *                dictated by the flow of the program
 *
 *                Finally, memory is the 64 KB storage space used by the 
 *                processor. It can be divided into a number of regions 
 *                depending on how you want to use it. Accessing memory will
 *                allways take considerably more cycles than accessing one of
 *                the three registers.
 * ============================================================================
 */
struct gem_mos {
    GEM_REGISTER8  a;   /* accumulator */
    GEM_REGISTER8  x,y; /* intermediate registers */
    GEM_REGISTER8  sr;  /* status register */
    GEM_REGISTER8  sp;  /* stack pointer */
    GEM_REGISTER16 pc;  /* program counter */

    GEM_MEMORY_BYTE memory[GEM_MEMORY_CAPACITY]; /* 64 KB memory for processor */
};

/*---------------------------------------------------------------------------
 *
 *  Emulator interface. Typical user of this software should only need to
 *  use these functions
 *
 *---------------------------------------------------------------------------*/

gem_mos*         gem_mos_new         ( void );
void             gem_mos_init        ( gem_mos *m );

void             gem_mos_sr_set      ( gem_mos *m, GEM_STATUS_FLAG flag );
void             gem_mos_sr_unset    ( gem_mos *m, GEM_STATUS_FLAG flag );
GEM_TEST         gem_mos_sr_test     ( gem_mos *m, GEM_STATUS_FLAG flag );
void             gem_mos_sr_update   ( gem_mos *m, GEM_STATUS_FLAG flag, 
                                                    GEM_TEST test );

GEM_MEMORY_WORD  gem_mos_mem_readw   ( gem_mos *m, GEM_MEMORY_ADDR addr );
GEM_MEMORY_BYTE  gem_mos_mem_read    ( gem_mos *m, GEM_MEMORY_ADDR addr );
void             gem_mos_mem_write   ( gem_mos *m, GEM_MEMORY_ADDR addr, 
                                                GEM_MEMORY_BYTE data );

int32_t          gem_mos_load_rom    ( gem_mos *m, char *fname );
int32_t          gem_mos_load_rom_at ( gem_mos *m, char *fname, 
                                                GEM_MEMORY_ADDR dest );

GEM_MEMORY_BYTE  gem_mos_sp_pop      ( gem_mos *m );
void             gem_mos_sp_push     ( gem_mos *m, GEM_MEMORY_BYTE data );

GEM_OPCODE       gem_mos_fetch       ( gem_mos *m, GEM_MEMORY_ADDR );
GEM_CLOCK_TICKS  gem_mos_execute     ( gem_mos *m, GEM_OPCODE code );
GEM_CLOCK_TICKS  gem_mos_step        ( gem_mos *m ); 
GEM_CLOCK_TICKS  gem_mos_run_for     ( gem_mos *m, 
                                                GEM_CLOCK_TICKS num_cycles );
void             gem_mos_reset_soft  ( gem_mos *m );

GEM_OPCODE       gem_get_opcode      ( void );
char*            gem_get_disasm      ( void );

void             gem_mos_free        ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x00
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_brk_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_asl_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_php_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_asl_acc   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_asl_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x10
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bpl_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_asl_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_clc_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ora_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_asl_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x20
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_jsr_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_bit_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_rol_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_plp_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_rol_acc   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_bit_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_rol_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x30
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bmi_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_rol_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sec_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_and_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_rol_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x40
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_rti_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_pha_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lsr_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lsr_acc   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_jmp_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lsr_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x50
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bvc_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lsr_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cli_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_eor_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lsr_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x60
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_rts_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ror_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_pla_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ror_acc   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_jmp_ind   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ror_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x70
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bvs_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ror_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sei_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_adc_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ror_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x80
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_sta_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sty_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_stx_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dey_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_txa_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sty_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_stx_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0x90
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bcc_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sty_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_stx_zpy   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_tya_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_txs_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sta_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xA0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_ldy_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldx_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldy_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldx_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_tay_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_tax_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldy_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldx_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xB0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bcs_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldy_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldx_zpy   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_clv_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_tsx_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldy_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_lda_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_ldx_aby   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xC0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_cpy_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cpy_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dec_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_iny_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dex_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cpy_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dec_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xD0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_bne_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dec_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cld_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cmp_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_dec_abx   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xE0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_cpx_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_xin   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cpx_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_inc_zpg   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_inx_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_imm   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_nop_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_cpx_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_abs   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_inc_abs   ( gem_mos *m );

/*---------------------------------------------------------------------------
 *
 *  opcode 0xF0
 *
 *---------------------------------------------------------------------------*/
GEM_CLOCK_TICKS  gem_mos_beq_rel   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_iny   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_inc_zpx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sed_imp   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_aby   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_sbc_abx   ( gem_mos *m );
GEM_CLOCK_TICKS  gem_mos_inc_abx   ( gem_mos *m );

#endif /* _GEM_MOS_H_ */
