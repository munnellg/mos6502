#include "gem_mos.h"

GEM_OPCODE  opcode = 0;
char disasm[256] = {0};


/* TODO: Switch statements are apparently converted to jump tables when 
 * compiled in C. Might be worth swapping this array out for a function call
 * with a big switch. Will remove the risk of hitting NULL array values. Also
 * makes it very easy to define default invalid opcode behaviour 
 *
 * Technically an emulator should really just be a big switch anyway. No need
 * for a load of function calls. From an emulation standpoint, I don't think
 * I'm doing this in the "traditional" style
 * */
static gem_instruction jump_table [] = {
    gem_mos_brk_imp, /* 0x00 */
    gem_mos_ora_xin, /* 0x01 */
    NULL, /* 0x02 */
    NULL, /* 0x03 */
    NULL, /* 0x04 */
    gem_mos_ora_zpg, /* 0x05 */
    gem_mos_asl_zpg, /* 0x06 */
    NULL, /* 0x07 */
    gem_mos_php_imp, /* 0x08 */
    gem_mos_ora_imm, /* 0x09 */
    gem_mos_asl_acc, /* 0x0A */
    NULL, /* 0x0B */
    NULL, /* 0x0C */
    gem_mos_ora_abs, /* 0x0D */
    gem_mos_asl_abs, /* 0x0E */
    NULL, /* 0x0F */

    gem_mos_bpl_rel, /* 0x10 */
    gem_mos_ora_iny, /* 0x11 */
    NULL, /* 0x12 */
    NULL, /* 0x13 */
    NULL, /* 0x14 */
    gem_mos_ora_zpx, /* 0x15 */
    gem_mos_asl_zpx, /* 0x16 */
    NULL, /* 0x17 */
    gem_mos_clc_imp, /* 0x18 */
    gem_mos_ora_aby, /* 0x19 */
    NULL, /* 0x1A */
    NULL, /* 0x1B */
    NULL, /* 0x1C */
    gem_mos_ora_abx, /* 0x1D */
    gem_mos_asl_abx, /* 0x1E */
    NULL, /* 0x1F */

    gem_mos_jsr_abs, /* 0x20 */
    gem_mos_and_xin, /* 0x21 */
    NULL, /* 0x22 */
    NULL, /* 0x23 */
    gem_mos_bit_zpg, /* 0x24 */
    gem_mos_and_zpg, /* 0x25 */
    gem_mos_rol_zpg, /* 0x26 */
    NULL, /* 0x27 */
    gem_mos_plp_imp, /* 0x28 */
    gem_mos_and_imm, /* 0x29 */
    gem_mos_rol_acc, /* 0x2A */
    NULL, /* 0x2B */
    gem_mos_bit_abs, /* 0x2C */
    gem_mos_and_abs, /* 0x2D */
    gem_mos_rol_abs, /* 0x2E */
    NULL, /* 0x2F */

    gem_mos_bmi_rel, /* 0x30 */
    gem_mos_and_iny, /* 0x31 */
    NULL, /* 0x32 */
    NULL, /* 0x33 */
    NULL, /* 0x34 */
    gem_mos_and_zpx, /* 0x35 */
    gem_mos_rol_zpx, /* 0x36 */
    NULL, /* 0x37 */
    gem_mos_sec_imp, /* 0x38 */
    gem_mos_and_aby, /* 0x39 */
    NULL, /* 0x3A */
    NULL, /* 0x3B */
    NULL, /* 0x3C */
    gem_mos_and_abx, /* 0x3D */
    gem_mos_rol_abx, /* 0x3E */
    NULL, /* 0x3F */
    
    gem_mos_rti_imp, /* 0x40 */
    gem_mos_eor_xin, /* 0x41 */
    NULL, /* 0x42 */
    NULL, /* 0x43 */
    NULL, /* 0x44 */
    gem_mos_eor_zpg, /* 0x45 */
    gem_mos_lsr_zpg, /* 0x46 */
    NULL, /* 0x47 */
    gem_mos_pha_imp, /* 0x48 */
    gem_mos_eor_imm, /* 0x49 */
    gem_mos_lsr_acc, /* 0x4A */
    NULL, /* 0x4B */
    gem_mos_jmp_abs, /* 0x4C */
    gem_mos_eor_abs, /* 0x4D */
    gem_mos_lsr_abs, /* 0x4E */
    NULL, /* 0x4F */

    gem_mos_bvc_rel, /* 0x50 */
    gem_mos_eor_iny, /* 0x51 */
    NULL, /* 0x52 */
    NULL, /* 0x53 */
    NULL, /* 0x54 */
    gem_mos_eor_zpx, /* 0x55 */
    gem_mos_lsr_zpx, /* 0x56 */
    NULL, /* 0x57 */
    gem_mos_cli_imp, /* 0x58 */
    gem_mos_eor_aby, /* 0x59 */
    NULL, /* 0x5A */
    NULL, /* 0x5B */
    NULL, /* 0x5C */
    gem_mos_eor_abx, /* 0x5D */
    gem_mos_lsr_abx, /* 0x5E */
    NULL, /* 0x5F */

    gem_mos_rts_imp, /* 0x60 */
    gem_mos_adc_xin, /* 0x61 */
    NULL, /* 0x62 */
    NULL, /* 0x63 */
    NULL, /* 0x64 */
    gem_mos_adc_zpg, /* 0x65 */
    gem_mos_ror_zpg, /* 0x66 */
    NULL, /* 0x67 */
    gem_mos_pla_imp, /* 0x68 */
    gem_mos_adc_imm, /* 0x69 */
    gem_mos_ror_acc, /* 0x6A */
    NULL, /* 0x6B */
    gem_mos_jmp_ind, /* 0x6C */
    gem_mos_adc_abs, /* 0x6D */
    gem_mos_ror_abs, /* 0x6E */
    NULL, /* 0x6F */

    gem_mos_bvs_rel, /* 0x70 */
    gem_mos_adc_iny, /* 0x71 */
    NULL, /* 0x72 */
    NULL, /* 0x73 */
    NULL, /* 0x74 */
    gem_mos_adc_zpx, /* 0x75 */
    gem_mos_ror_zpx, /* 0x76 */
    NULL, /* 0x77 */
    gem_mos_sei_imp, /* 0x78 */
    gem_mos_adc_aby, /* 0x79 */
    NULL, /* 0x7A */
    NULL, /* 0x7B */
    NULL, /* 0x7C */
    gem_mos_adc_abx, /* 0x7D */
    gem_mos_ror_abx, /* 0x7E */
    NULL, /* 0x7F */

    NULL, /* 0x80 */
    gem_mos_sta_xin, /* 0x81 */
    NULL, /* 0x82 */
    NULL, /* 0x83 */
    gem_mos_sty_zpg, /* 0x84 */
    gem_mos_sta_zpg, /* 0x85 */
    gem_mos_stx_zpg, /* 0x86 */
    NULL, /* 0x87 */
    gem_mos_dey_imp, /* 0x88 */
    NULL, /* 0x89 */
    gem_mos_txa_imp, /* 0x8A */
    NULL, /* 0x8B */
    gem_mos_sty_abs, /* 0x8C */
    gem_mos_sta_abs, /* 0x8D */
    gem_mos_stx_abs, /* 0x8E */
    NULL, /* 0x8F */

    gem_mos_bcc_rel, /* 0x90 */
    gem_mos_sta_iny, /* 0x91 */
    NULL, /* 0x92 */
    NULL, /* 0x93 */
    gem_mos_sty_zpx, /* 0x94 */
    gem_mos_sta_zpx, /* 0x95 */
    gem_mos_stx_zpy, /* 0x96 */
    NULL, /* 0x97 */
    gem_mos_tya_imp, /* 0x98 */
    gem_mos_sta_aby, /* 0x99 */
    gem_mos_txs_imp, /* 0x9A */
    NULL, /* 0x9B */
    NULL, /* 0x9C */
    gem_mos_sta_abx, /* 0x9D */
    NULL, /* 0x9E */
    NULL, /* 0x9F */

    gem_mos_ldy_imm, /* 0xA0 */
    gem_mos_lda_xin, /* 0xA1 */
    gem_mos_ldx_imm, /* 0xA2 */
    NULL, /* 0xA3 */
    gem_mos_ldy_zpg, /* 0xA4 */
    gem_mos_lda_zpg, /* 0xA5 */
    gem_mos_ldx_zpg, /* 0xA6 */
    NULL, /* 0xA7 */
    gem_mos_tay_imp, /* 0xA8 */
    gem_mos_lda_imm, /* 0xA9 */
    gem_mos_tax_imp, /* 0xAA */
    NULL, /* 0xAB */
    gem_mos_ldy_abs, /* 0xAC */
    gem_mos_lda_abs, /* 0xAD */
    gem_mos_ldx_abs, /* 0xAE */
    NULL, /* 0xAF */

    gem_mos_bcs_rel, /* 0xB0 */
    gem_mos_lda_iny, /* 0xB1 */
    NULL, /* 0xB2 */
    NULL, /* 0xB3 */
    gem_mos_ldy_zpx, /* 0xB4 */
    gem_mos_lda_zpx, /* 0xB5 */
    gem_mos_ldx_zpy, /* 0xB6 */
    NULL, /* 0xB7 */
    gem_mos_clv_imp, /* 0xB8 */
    gem_mos_lda_aby, /* 0xB9 */
    gem_mos_tsx_imp, /* 0xBA */
    NULL, /* 0xBB */
    gem_mos_ldy_abx, /* 0xBC */
    gem_mos_lda_abx, /* 0xBD */
    gem_mos_ldx_aby, /* 0xBE */
    NULL, /* 0xBF */

    gem_mos_cpy_imm, /* 0xC0 */
    gem_mos_cmp_xin, /* 0xC1 */
    NULL, /* 0xC2 */
    NULL, /* 0xC3 */
    gem_mos_cpy_zpg, /* 0xC4 */
    gem_mos_cmp_zpg, /* 0xC5 */
    gem_mos_dec_zpg, /* 0xC6 */
    NULL, /* 0xC7 */
    gem_mos_iny_imp, /* 0xC8 */
    gem_mos_cmp_imm, /* 0xC9 */
    gem_mos_dex_imp, /* 0xCA */
    NULL, /* 0xCB */
    gem_mos_cpy_abs, /* 0xCC */
    gem_mos_cmp_abs, /* 0xCD */
    gem_mos_dec_abs, /* 0xCE */
    NULL, /* 0xCF */

    gem_mos_bne_rel, /* 0xD0 */
    gem_mos_cmp_iny, /* 0xD1 */
    NULL, /* 0xD2 */
    NULL, /* 0xD3 */
    NULL, /* 0xD4 */
    gem_mos_cmp_zpx, /* 0xD5 */
    gem_mos_dec_zpx, /* 0xD6 */
    NULL, /* 0xD7 */
    gem_mos_cld_imp, /* 0xD8 */
    gem_mos_cmp_aby, /* 0xD9 */
    NULL, /* 0xDA */
    NULL, /* 0xDB */
    NULL, /* 0xDC */
    gem_mos_cmp_abx, /* 0xDD */
    gem_mos_dec_abx, /* 0xDE */
    NULL, /* 0xDF */

    gem_mos_cpx_imm, /* 0xE0 */
    gem_mos_sbc_xin, /* 0xE1 */
    NULL, /* 0xE2 */
    NULL, /* 0xE3 */
    gem_mos_cpx_zpg, /* 0xE4 */
    gem_mos_sbc_zpg, /* 0xE5 */
    gem_mos_inc_zpg, /* 0xE6 */
    NULL, /* 0xE7 */
    gem_mos_inx_imp, /* 0xE8 */
    gem_mos_sbc_imm, /* 0xE9 */
    gem_mos_nop_imp, /* 0xEA */
    NULL, /* 0xEB */
    gem_mos_cpx_abs, /* 0xEC */
    gem_mos_sbc_abs, /* 0xED */
    gem_mos_inc_abs, /* 0xEE */
    NULL, /* 0xEF */

    gem_mos_beq_rel, /* 0xF0 */
    gem_mos_sbc_iny, /* 0xF1 */
    NULL, /* 0xF2 */
    NULL, /* 0xF3 */
    NULL, /* 0xF4 */
    gem_mos_sbc_zpx, /* 0xF5 */
    gem_mos_inc_zpx, /* 0xF6 */
    NULL, /* 0xF7 */
    gem_mos_sed_imp, /* 0xF8 */
    gem_mos_sbc_aby, /* 0xF9 */
    NULL, /* 0xFA */
    NULL, /* 0xFB */
    NULL, /* 0xFC */
    gem_mos_sbc_abx, /* 0xFD */
    gem_mos_inc_abx, /* 0xFE */
    NULL, /* 0xFF */
};

/*---------------------------------------------------------------------------
 *  Addressing modes for the MOS 6502. 
 *
 *  These affect how the arguments to a given instruction are retrieved. With 
 *  the exception of the immediate mode (which actually returns the argument 
 *  itself), these functions all return the memory address of the argument so 
 *  that it may be fetched by the instruction.
 *
 *  They will also update the Program Counter as required.
 *---------------------------------------------------------------------------*/

/*
 * ===  FUNCTION  =============================================================
 *          Name: absolute
 *  Desecription: Retrieves an absolute address of argument from memory. The 
 *                absolute address is stored in the two bytes following the 
 *                opcode with the low byte being stored at Program Counter + 1 
 *                and the high byte being stored at Program Counter + 2
 *
 *                This function retrieves the address from memory based on the
 *                current value of the program counter. It also updates the 
 *                value of the program counter so that it points to the byte
 *                just after the address of the high byte.
 *
 *                The absolute mode can address the full 64 KB length of the 
 *                MOS 6502's memory
 * ============================================================================
 */
static GEM_MEMORY_ADDR
absolute ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr;    
    
    /* an absolute address is 2 bytes wide, so we use gem_mos_mem_readw to read 
     * a word */
    addr = gem_mos_mem_readw( m, m->pc );

    /* advance the program counter so that it points to the instruction after
     * the absolute address. The absolute address is 2 bytes wide, so add 2 */
    m->pc += 2;

    /* return the address of the argument */
    return addr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: absolute_x
 *  Desecription: This is the x-indexed absolute addressing mode. Its operation
 *                is similar to the absolute addressing mode except that we
 *                add the value of the X register and the CARRY_FLAG to the 
 *                address retrieved from memory.
 *
 *                For simplicity, this function simply calls the absolute
 *                function above and returns the result plus the X register
 *                plus the current value of the CARRY_FLAG.
 *
 *                This function should incur an extra time penalty if we cross
 *                a page boundary when indexing
 *
 *                Updating the Program Counter is handled by the absolute
 *                function
 * ============================================================================
 */
static GEM_MEMORY_ADDR
absolute_x ( gem_mos *m ) {
    /* TODO: Check for page crossings */
    return absolute(m) + m->x;// + gem_mos_sr_test( m, SR_FLAG_CARRY );
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: absolute_y
 *  Desecription: This is the y-indexed absolute addressing mode. Its operation
 *                is similar to the absolute addressing mode except that we
 *                add the value of the Y register and the CARRY_FLAG to the
 *                address retrieved from memory
 *
 *                For simplicity, this function simply calls the absolute
 *                function above and returns the result plus the Y register
 *                plus the current value of the CARRY_FLAG.
 *
 *                This function should incur an extra time penalty if we cross
 *                a page boundary when indexing
 *
 *                Updating the Program Counter is handled by the absolute 
 *                function
 * ============================================================================
 */
static GEM_MEMORY_ADDR
absolute_y ( gem_mos *m ) {
    /* TODO: Check for page crossings */
    return absolute(m) + m->y;// + gem_mos_sr_test( m, SR_FLAG_CARRY );
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: immediate
 *  Desecription: This is the only addressing mode which returns a value
 *                rather than an address. When using the immediate addressing
 *                mode, the byte following the instruction contains the value
 *                of the argument to the instruction. This value can simply
 *                be returned and operated on.
 *
 *                The program counter is updated as per normal so that it 
 *                points to the instruction immediately after the argument
 * ============================================================================
 */
static GEM_MEMORY_BYTE
immediate ( gem_mos *m ) {
    return gem_mos_mem_read(m, m->pc++);
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: indirect 
 *  Desecription: This is a somewhat strange addressing mode. The two bytes
 *                following the instruction contain the address of the address
 *                of the argument. In other words, we have a pointer to a 
 *                pointer.
 *
 *                For the sake of terminology, the two bytes following the 
 *                instruction contain the **effective** address. The value 
 *                pointed to by the effective address is the address of the
 *                argument.
 *
 *                This function uses the absolute addressing mode function
 *                above to retrieve the effective address. It then uses
 *                the gem_mos_mem_readw function to retrieve the address of
 *                the argument based on the effective address.
 *
 *                Updating the program counter is handled by the absolute
 *                function
 * ============================================================================
 */
static GEM_MEMORY_ADDR
indirect ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr, eaddr;
    eaddr = absolute(m); 
    addr = gem_mos_mem_readw( m, eaddr );
    return addr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR 
indirect_x ( gem_mos *m ) {
    GEM_MEMORY_BYTE addr;
    GEM_MEMORY_ADDR eaddr;
    addr = gem_mos_mem_read( m, m->pc++ );
    addr += m->x;
    eaddr = gem_mos_mem_readw( m, addr );
    return eaddr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR 
indirect_y ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr, eaddr;
    eaddr = gem_mos_mem_read( m, m->pc++ );  
    addr = gem_mos_mem_readw( m, eaddr );
    return addr + m->y;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: zeropage
 *  Desecription: The zeropage addressing mode can be used to address the first
 *                256 bytes of MOS 6502 memory. It is a little faster than
 *                using the absolute addressing mode because we only need to
 *                read the low byte of the address from memory. The high byte
 *                is assumed to be zero.
 *
 *                The value returned by this function will be an address 
 *                somewhere between 0x0000 and 0x00FF
 * ============================================================================
 */
static GEM_MEMORY_ADDR
zeropage ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr;    
    addr = gem_mos_mem_read( m, m->pc++ );
    return addr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR
zeropage_x ( gem_mos *m ) {
    GEM_MEMORY_BYTE addr;    
    addr = gem_mos_mem_read( m, m->pc++ );
    addr += m->x;
    return (GEM_MEMORY_ADDR) addr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR
zeropage_y ( gem_mos *m ) {
    GEM_MEMORY_BYTE addr;    
    addr = gem_mos_mem_read( m, m->pc++ );
    addr += m->y;
    return (GEM_MEMORY_ADDR) addr;
}

static GEM_OFFSET
relative ( gem_mos *m ) {
    return m->memory[m->pc++];
}

static void
add_with_carry ( gem_mos *m, GEM_REGISTER8 b ) {
    GEM_REGISTER16 x;
    GEM_REGISTER8 bcd_lo;

    /* compute result */
    x = m->a + b + gem_mos_sr_test( m, SR_FLAG_CARRY );
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(x) );

    if( gem_mos_sr_test( m, SR_FLAG_DECIMAL ) ) {
        bcd_lo = GEM_GET_LNIBBLE( m->a ) + GEM_GET_LNIBBLE( b ) 
            + gem_mos_sr_test( m, SR_FLAG_CARRY );
        
        gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(x) );  
        gem_mos_sr_update( m, SR_FLAG_OVERFLOW, GEM_TEST_OVERFLOW( m->a, b, x ) );
        
        if( bcd_lo > 0x09 ) {
            x += 0x06;
        }
        
        if( x > 0x99 ) {
            x += 0x60;
        }
        
        gem_mos_sr_update( m, SR_FLAG_CARRY, x > 0x99 );   
        
    } else {
        gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(x) );  
        gem_mos_sr_update( m, SR_FLAG_OVERFLOW, GEM_TEST_OVERFLOW( m->a, b, x ) );
        gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_CARRY( m->a, b,
                   GEM_REGISTER8_MAX - gem_mos_sr_test( m, SR_FLAG_CARRY)) );   
    }

    /* move result to accumulator */
    m->a = x;
}

static void
and ( gem_mos *m, GEM_REGISTER8 x ) {
    m->a &= x;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->a) );
}

static void
arithmetic_shift_left ( gem_mos *m, GEM_REGISTER8 *a ) {
    gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_MSB(*a) );
    *a <<= 1;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
bit ( gem_mos *m, GEM_MEMORY_BYTE x ) {
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, x & SR_FLAG_NEGATIVE );
    gem_mos_sr_update( m, SR_FLAG_OVERFLOW, x & SR_FLAG_OVERFLOW );
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(x&m->a) );
}

static void
branch ( gem_mos *m, GEM_OFFSET o, GEM_TEST condition ) {
    if(condition) {
        m->pc += o;
    }
}

static void
compare ( gem_mos *m, GEM_MEMORY_BYTE x, GEM_MEMORY_BYTE y ) {
    GEM_MEMORY_BYTE result;
    result = x + (~y + 1);
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(result) );                    
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(result) );
    gem_mos_sr_update( m, SR_FLAG_CARRY, x >= y ); 
            //GEM_TEST_CARRY( comp, 1, GEM_REGISTER8_MAX) );  
}

static void
decrement ( gem_mos *m, GEM_REGISTER8 *x ) {
    (*x)--;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*x) );  

}

static void
eor ( gem_mos *m, GEM_REGISTER8 x ) {
    m->a ^= x;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->a) );
}

static void
increment ( gem_mos *m, GEM_REGISTER8 *x ) {
    (*x)++;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*x) );  
}

static void
jump ( gem_mos *m, GEM_MEMORY_ADDR t ) {
    m->pc = t;
}

static void
logical_shift_right ( gem_mos *m, GEM_REGISTER8 *a ) {
    gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_LSB(*a) );
    *a >>= 1;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
or ( gem_mos *m, GEM_MEMORY_BYTE b ) {
    m->a |= b;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->a) );
}

static void
rotate_left ( gem_mos *m, GEM_REGISTER8 *a ) {
    GEM_TEST tmp = gem_mos_sr_test( m, SR_FLAG_CARRY );
    gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_MSB(*a) );
    *a <<= 1;
    *a |= tmp;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
rotate_right ( gem_mos *m, GEM_REGISTER8 *a ) {
    GEM_TEST tmp = gem_mos_sr_test( m, SR_FLAG_CARRY );
    gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_LSB(*a) );
    *a >>= 1;
    *a |= (tmp << ((sizeof(GEM_REGISTER8) * CHAR_BIT)-1));
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
subtract_with_carry ( gem_mos *m, GEM_REGISTER8 b ) {
    GEM_REGISTER16 x;

    x = m->a - b - ( 1 - gem_mos_sr_test( m, SR_FLAG_CARRY )) ;

    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(x) );   
    gem_mos_sr_update( m, SR_FLAG_OVERFLOW, 
            GEM_TEST_NEGATIVE( (m->a ^ x) ) && GEM_TEST_NEGATIVE( m->a ^ b ) );

    if( gem_mos_sr_test( m, SR_FLAG_DECIMAL ) ) {
        if( GEM_GET_LNIBBLE( m->a ) - ( 1 - gem_mos_sr_test(m, SR_FLAG_CARRY))
               < GEM_GET_LNIBBLE(b) ) {
            x -= 0x06;
        }
        if( x > 0x99 ) {
            x -= 0x60;
        } 
    }

    gem_mos_sr_update( m, SR_FLAG_CARRY, x < 0x100 );   

    m->a = x;
}

static void
transfer ( gem_mos *m, GEM_REGISTER8 *a, GEM_REGISTER8 b, GEM_TEST update ) {
    *a = b;
    if(update) {
        gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
        gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
    }
}

gem_mos*
gem_mos_new ( void ) {
    gem_mos *m;

    m = malloc( sizeof(gem_mos) );

    if(m) {
        gem_mos_init(m);
    }
    
    return m;
}

void
gem_mos_init ( gem_mos *m ) {
    m->a  = 0;
    m->x  = 0;
    m->y  = 0;
    m->sp = 0xFF;
    m->sr = 0;
    m->pc = GEM_ROM_ADDR;
}

void
gem_mos_sr_set   ( gem_mos *m, GEM_STATUS_FLAG flag ) {
    m->sr |= flag;
}

void
gem_mos_sr_unset ( gem_mos *m, GEM_STATUS_FLAG flag ) {
    m->sr &= ~flag;
}

void
gem_mos_sr_update ( gem_mos *m, GEM_STATUS_FLAG flag, GEM_TEST test ) {
    if(test) {
        gem_mos_sr_set( m, flag );
    } else {
        gem_mos_sr_unset( m, flag );
    }
}

GEM_TEST
gem_mos_sr_test ( gem_mos *m, GEM_STATUS_FLAG flag ) {
    return (m->sr & flag) != 0;
}

GEM_MEMORY_WORD
gem_mos_mem_readw ( gem_mos *m, GEM_MEMORY_ADDR addr ) {
    GEM_MEMORY_WORD data;    
    data = gem_mos_mem_read( m, addr++ );
    data |= ((GEM_MEMORY_WORD)gem_mos_mem_read( m, addr )) << 8;
    return data;
}

GEM_MEMORY_BYTE
gem_mos_mem_read ( gem_mos *m, GEM_MEMORY_ADDR addr ) {
    return m->memory[addr];
}

void
gem_mos_mem_write ( gem_mos *m, GEM_MEMORY_ADDR addr, GEM_MEMORY_BYTE data ) {
    m->memory[addr] = data;
}

int32_t
gem_mos_load_rom ( gem_mos *m, char *fname ) {
    return gem_mos_load_rom_at( m, fname, GEM_ROM_ADDR ); 
}

int32_t
gem_mos_load_rom_at ( gem_mos *m, char *fname, GEM_MEMORY_ADDR dest ) {
    FILE *f;
    int32_t addr;

    f = fopen(fname, "r");

    if(!f) {
        return -1;
    }
    
    addr = dest;
    while(!feof(f) && addr < GEM_MEMORY_CAPACITY ) {
        m->memory[addr++] = getc(f);
    }

    fclose(f);

    return addr - dest; 
}

GEM_MEMORY_ADDR
gem_mos_sp_popw ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr, word;
    addr = GEM_MEMORY_STACK +  ++(m->sp);
    word = m->memory[addr+1];
    word = (word << 8) | m->memory[addr];
    m->sp++;
    return word;
}

GEM_MEMORY_BYTE
gem_mos_sp_pop ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr;
    addr = GEM_MEMORY_STACK +  ++(m->sp);
    return m->memory[addr];
}

void
gem_mos_sp_push ( gem_mos *m, GEM_MEMORY_BYTE data ) {
    GEM_MEMORY_ADDR addr;
    addr = GEM_MEMORY_STACK + (m->sp)--;
    m->memory[addr] = data;
}

/* fetch an instruction opcode from mos memory and return the value of the 
 * opcode */
GEM_OPCODE
gem_mos_fetch ( gem_mos *m, GEM_MEMORY_ADDR addr ) {
    return m->memory[addr];
}

/* execute opcode and return the number of cpu cycles required for execution */
GEM_CLOCK_TICKS
gem_mos_execute ( gem_mos *m, GEM_OPCODE code ) {
    if( jump_table[code] ) {
        return jump_table[code](m); 
    } else {
        return -1;
    }
}

/* fetch and execute next CPU instruction */
GEM_CLOCK_TICKS
gem_mos_step ( gem_mos *m ) {
    GEM_OPCODE code;

    code = gem_mos_fetch(m, m->pc++);
    opcode = code;
    return gem_mos_execute( m, code );
}

GEM_CLOCK_TICKS
gem_mos_run_for ( gem_mos *m, GEM_CLOCK_TICKS num_cycles ) {
    GEM_CLOCK_TICKS executed;
    int result;

    executed = 0;
    result = 0;

    while ( executed < num_cycles && result >= 0 ) {
        if( result >= 0 ) {
            executed += gem_mos_step(m);
        }
    }

    return (result < 0)? -executed : executed;
}

void
gem_mos_reset_soft ( gem_mos *m ) {
    m->pc = m->memory[GEM_VECTOR_RESET+1];
    m->pc <<= 8;
    m->pc |= m->memory[GEM_VECTOR_RESET];   
}

GEM_OPCODE
gem_get_opcode ( void ) {
    return opcode;
}

char*
gem_get_disasm ( void ) {
    return disasm;
}

void
gem_mos_free ( gem_mos *m ) {
    if(m) {
        free(m);
    }
}

/* TODO: Devise a nicer way to compute ticks for each instruction */

GEM_CLOCK_TICKS
gem_mos_brk_imp ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 6;    
    gem_mos_sp_push( m, GEM_GET_HBYTE(m->pc+1) );
    gem_mos_sp_push( m, GEM_GET_LBYTE(m->pc+1) );
    arg = gem_mos_mem_readw(m, GEM_VECTOR_BRK );
    gem_mos_sr_set( m, SR_FLAG_BREAK | SR_FLAG_UNUSED ); 
    gem_mos_sp_push( m, m->sr );
    gem_mos_sr_set( m, SR_FLAG_INTERRUPT );
    jump( m, arg );
    sprintf(disasm, "BRK $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_REGISTER8 arg;
    ticks = 3;        
    addr = indirect_x(m);
    arg = gem_mos_mem_read( m, addr );
    or(m, arg);
    sprintf(disasm, "ORA ($%02X,X) ; 0x%02X", addr - m->x, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_REGISTER8 arg;
    ticks = 3;      
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA $%02X ; 0x%02X", addr&0xFF, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;      
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &arg );  
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "ASL $%02X ; 0x%02X", addr&0xFF, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_php_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 3;
    gem_mos_sr_set( m, SR_FLAG_BREAK | SR_FLAG_UNUSED );
    gem_mos_sp_push( m, m->sr );    
    sprintf(disasm, "PHP");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_REGISTER8 arg;
    ticks = 3;
    arg = immediate(m);
    or( m, arg );
    sprintf(disasm, "ORA #$%02X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_acc ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;          
    arithmetic_shift_left( m, &m->a );  
    sprintf(disasm, "ASL A");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_REGISTER8 arg;
    ticks = 4;
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA $%04X ; 0x%02X", addr, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;      
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &arg );  
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "ASL $%04X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bpl_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, !gem_mos_sr_test( m, SR_FLAG_NEGATIVE ) );
    sprintf(disasm, "BPL $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = indirect_y(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA ($%02X), Y ; 0x%02X", addr, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA $%02X, X ; 0x%02X", addr-m->x, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;      
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &arg );  
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "ASL $%02X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_clc_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_CARRY );   
    sprintf(disasm, "CLC" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_y(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA $%04X, Y ; 0x%02X", addr-m->x, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    or( m, arg );
    sprintf(disasm, "ORA $%04X, X ; 0x%02X", addr-m->x, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;      
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &arg );  
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "ASL $%04X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_jsr_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 6;    
    gem_mos_sp_push( m, GEM_GET_HBYTE(m->pc+1) );
    gem_mos_sp_push( m, GEM_GET_LBYTE(m->pc+1) );
    arg = absolute(m);
    jump( m, arg );
    sprintf(disasm, "JSR $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;        
    addr = indirect_x(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND ($%02X,X) ; 0x%02X", addr - m->x, arg); 
    return ticks;
}

GEM_CLOCK_TICKS 
gem_mos_bit_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    GEM_MEMORY_ADDR addr;
    ticks = 3; 
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    bit ( m, arg );
    sprintf(disasm, "BIT $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;        
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND $%02X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rol_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = zeropage(m);
    arg = gem_mos_mem_read(m, addr);
    rotate_left( m, &arg );
    gem_mos_mem_write(m, addr, arg);
    sprintf( disasm, "ROL $%02X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_plp_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    m->sr = gem_mos_sp_pop( m );    
    sprintf(disasm, "PLP" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_BYTE arg;
    ticks = 2;        
    arg = immediate(m);
    and(m, arg);
    sprintf(disasm, "AND #$%02X", arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rol_acc ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    rotate_left( m, &m->a );
    sprintf( disasm, "ROL A" );
    return ticks;
}

GEM_CLOCK_TICKS 
gem_mos_bit_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4; 
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    bit ( m, arg );
    sprintf(disasm, "BIT $%04X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND $%04X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rol_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = absolute(m);
    arg = gem_mos_mem_read(m, addr);
    rotate_left( m, &arg );
    gem_mos_mem_write(m, addr, arg);
    sprintf( disasm, "ROL $%04X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bmi_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, gem_mos_sr_test( m, SR_FLAG_NEGATIVE ) );
    sprintf(disasm, "BMI $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;        
    addr = indirect_y(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND ($%02X), Y ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND $%02X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rol_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read(m, addr);
    rotate_left( m, &arg );
    gem_mos_mem_write(m, addr, arg);
    sprintf( disasm, "ROL $%02X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sec_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_CARRY ); 
    sprintf(disasm, "SEC" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute_y(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND $%04X, Y ; 0x%02X", addr - m->y, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_and_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    and(m, arg);
    sprintf(disasm, "AND $%04X, X ; 0x%02X", addr - m->x, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rol_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;
    addr = absolute_x(m);
    arg = gem_mos_mem_read(m, addr);
    rotate_left( m, &arg );
    gem_mos_mem_write(m, addr, arg);
    sprintf( disasm, "ROL $%04X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rti_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 6;
    m->sr = gem_mos_sp_pop( m );
    m->pc = gem_mos_sp_popw( m );
    sprintf( disasm, "RTI" );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;        
    addr = indirect_x(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR ($%02X, X) ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;        
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR $%02X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lsr_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    logical_shift_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "LSR $%02X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_pha_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 3;
    gem_mos_sp_push( m, m->a );
    sprintf(disasm, "PHA" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_imm ( gem_mos *m ) {
    GEM_MEMORY_BYTE arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    arg = immediate(m);
    eor( m, arg );
    sprintf(disasm, "EOR #$%02X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lsr_acc ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    logical_shift_right( m, &m->a );
    sprintf( disasm, "LSR A" );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_jmp_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 3;    
    arg = absolute(m);
    jump( m, arg );
    sprintf(disasm, "JMP $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR $%04X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lsr_abs( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    logical_shift_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "LSR $%04X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bvc_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, !gem_mos_sr_test( m, SR_FLAG_OVERFLOW ) );
    sprintf(disasm, "BVC $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;        
    addr = indirect_y(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR ($%02X), Y ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR $%02X, X ; 0x%02X", addr-m->x, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lsr_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    logical_shift_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "LSR $%02X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cli_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_INTERRUPT );   
    sprintf(disasm, "CLI" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute_y(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR $%04X, Y ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_eor_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;        
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    eor(m, arg);
    sprintf(disasm, "EOR $%04X, X ; 0x%02X", addr, arg); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lsr_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    logical_shift_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "LSR $%04X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_rts_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;    
    ticks = 6;    
    m->pc = gem_mos_sp_popw(m);
    jump( m, ++m->pc );
    sprintf(disasm, "RTS" );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = indirect_x( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC ($%02X, X) ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ror_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    rotate_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "ROR $%02X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_pla_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    m->a = gem_mos_sp_pop( m );
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->a) );  
    sprintf(disasm, "PLA" );     
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_imm ( gem_mos *m ) {
    GEM_MEMORY_BYTE arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    arg = immediate(m);
    add_with_carry( m, arg );
    sprintf(disasm, "ADC #$%02X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ror_acc ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    rotate_right( m, &m->a );
    sprintf( disasm, "ROR A" );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_jmp_ind ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 5;    
    arg = indirect(m);
    jump( m, arg );
    sprintf(disasm, "JMP ($%04X)", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC $%04X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ror_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    rotate_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "ROR $%04X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bvs_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, gem_mos_sr_test( m, SR_FLAG_OVERFLOW ) );
    sprintf(disasm, "BVS $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = indirect_y( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC ($%02X), Y ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC $%02X, X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ror_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    rotate_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "ROR $%02X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sei_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_INTERRUPT ); 
    sprintf(disasm, "SEI" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_y( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC $%04X, Y ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_adc_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_x( m );
    arg = gem_mos_mem_read( m, addr );
    add_with_carry( m, arg );
    sprintf( disasm, "ADC $%04X, X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ror_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;
    addr = absolute_x(m);
    arg = gem_mos_mem_read( m, addr );
    rotate_right( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf( disasm, "ROR $%04X, X", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 6;
    addr = indirect_x(m);
    gem_mos_mem_write( m, addr, m->a );
    sprintf( disasm, "STA ($0x%02X, X)", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sty_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    ticks = 3;      
    addr = zeropage(m);
    gem_mos_mem_write( m, addr, m->y );
    sprintf(disasm, "STY $%02X", addr&0xFF);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    ticks = 3;      
    addr = zeropage(m);
    gem_mos_mem_write( m, addr, m->a );
    sprintf(disasm, "STA $%02X", addr&0xFF);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_stx_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    GEM_MEMORY_ADDR addr;
    ticks = 3;      
    addr = zeropage(m);
    gem_mos_mem_write( m, addr, m->x );
    sprintf(disasm, "STX $%02X", addr&0xFF);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dey_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    decrement( m, &m->y );
    sprintf(disasm, "DEY" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_txa_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->a, m->x, SR_UPDATE );
    sprintf(disasm, "TXA");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sty_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    arg = absolute(m);
    gem_mos_mem_write( m, arg, m->y );    
    sprintf(disasm, "STY $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    arg = absolute(m);
    gem_mos_mem_write( m, arg, m->a );    
    sprintf(disasm, "STA $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_stx_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    arg = absolute(m);
    gem_mos_mem_write( m, arg, m->x );    
    sprintf(disasm, "STX $%04X", arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bcc_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, !gem_mos_sr_test( m, SR_FLAG_CARRY ) );
    sprintf(disasm, "BCC $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 6;
    addr = indirect_y(m);
    gem_mos_mem_write( m, addr, m->a );
    sprintf( disasm, "STA ($0x%02X), Y", addr );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sty_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = zeropage_x(m);
    gem_mos_mem_write( m, addr, m->y );
    sprintf( disasm, "STY $%02X, X", addr - m->x );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = zeropage_x(m);
    gem_mos_mem_write( m, addr, m->a );
    sprintf( disasm, "STA $%02X, X", addr - m->x );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_stx_zpy ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = zeropage_y(m);
    gem_mos_mem_write( m, addr, m->x );
    sprintf( disasm, "STX $%02X, Y", addr - m->y );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tya_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->a, m->y, SR_UPDATE );
    sprintf(disasm, "TYA" );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 5;
    addr = absolute_y( m );
    gem_mos_mem_write( m, addr, m->a );
    sprintf( disasm, "STA $%04X, Y", addr - m->y );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_txs_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->sp, m->x, SR_NO_UPDATE );
    sprintf(disasm, "TXS");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 5;
    addr = absolute_x(m);
    gem_mos_mem_write( m, addr, m->a );
    sprintf( disasm, "STA $%04X, X", addr-m->x);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->y = immediate(m);
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->y) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->y) );  
    sprintf(disasm, "LDY #$%02X", m->y);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 6;
    addr = indirect_x(m);
    transfer(m, &m->a, gem_mos_mem_read(m, addr), SR_UPDATE );
    sprintf(disasm, "LDA ($%02X, X) ; 0x%02X", addr, m->a);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer( m, &m->x, immediate(m), SR_UPDATE);
    sprintf(disasm, "LDX #$%02X", m->x); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 3;
    addr = zeropage(m);
    transfer(m, &m->y, gem_mos_mem_read(m, addr), SR_UPDATE );
    sprintf(disasm, "LDY $%02X", addr);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    transfer(m, &m->a, arg, SR_UPDATE );
    sprintf(disasm, "LDA $%02X ; 0x%02X", addr, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 3;
    addr = zeropage(m);
    transfer(m, &m->x, gem_mos_mem_read(m, addr), SR_UPDATE);
    sprintf(disasm, "LDX $%02X", addr);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer( m, &m->a, immediate(m), SR_UPDATE );
    sprintf(disasm, "LDA #$%02X", m->a);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tax_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->x, m->a, SR_UPDATE ); 
    sprintf(disasm, "TAX");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    arg = absolute(m);
    transfer( m, &m->y, gem_mos_mem_read( m, arg ), SR_UPDATE );
    sprintf( disasm, "LDY $%04X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    arg = absolute(m);
    transfer( m, &m->a, gem_mos_mem_read( m, arg ), SR_UPDATE );
    sprintf( disasm, "LDA $%04X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR arg;
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    arg = absolute(m);
    transfer( m, &m->x, gem_mos_mem_read( m, arg ), SR_UPDATE );
    sprintf( disasm, "LDX $%04X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tay_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->y, m->a, SR_UPDATE );
    sprintf(disasm, "TAY");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bcs_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, gem_mos_sr_test( m, SR_FLAG_CARRY ) );
    sprintf(disasm, "BCS $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 5;
    addr = indirect_y(m);
    transfer(m, &m->a, gem_mos_mem_read(m, addr), SR_UPDATE );
    sprintf(disasm, "LDA ($%02X), Y ; 0x%02X", addr, m->a);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    transfer( m, &m->y, arg, SR_UPDATE );
    sprintf( disasm, "LDY $%02X, X; 0x%02X", addr-m->x, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x(m);
    arg = gem_mos_mem_read( m, addr );
    transfer( m, &m->a, arg, SR_UPDATE );
    sprintf( disasm, "LDA $%02X, X; 0x%02X", addr-m->x, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_zpy ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = zeropage_y( m );
    arg = gem_mos_mem_read( m, addr );
    transfer( m, &m->x, arg, SR_UPDATE );
    sprintf( disasm, "LDS $%02X, Y ; 0x%02X", addr-m->y, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_clv_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_OVERFLOW );    
    sprintf(disasm, "CLV");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = absolute_y(m);
    transfer( m, &m->a, gem_mos_mem_read( m, addr ), SR_UPDATE );
    sprintf( disasm, "LDA $%04X, Y ; 0x%02X", addr-m->y, m->a);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tsx_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->x, m->sp, SR_UPDATE );    
    sprintf(disasm, "TSX");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = absolute_x(m);
    transfer( m, &m->y, gem_mos_mem_read( m, addr ), SR_UPDATE );
    sprintf( disasm, "LDY $%04X, X ; 0x%02X", addr-m->x, m->y);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = absolute_x(m);
    transfer( m, &m->a, gem_mos_mem_read( m, addr ), SR_UPDATE );
    sprintf( disasm, "LDA $%04X, X ; 0x%02X", addr-m->x, m->a);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    ticks = 4;
    addr = absolute_y(m);
    transfer( m, &m->x, gem_mos_mem_read( m, addr ), SR_UPDATE );
    sprintf( disasm, "LDX $%04X, Y ; 0x%02X", addr-m->y, m->x);
    return ticks;
}



GEM_CLOCK_TICKS
gem_mos_cpy_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    ticks = 2;
    arg = immediate(m);
    compare( m, m->y, arg );
    sprintf( disasm, "CPY #$%02X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = indirect_x(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP ($%02X, X) ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cpy_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->y, arg );
    sprintf( disasm, "CPY $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dec_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = zeropage( m );
    arg = gem_mos_mem_read( m, addr );
    decrement( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "DEC $%02X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_iny_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    increment( m, &m->y );
    sprintf(disasm, "INY");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    ticks = 2;
    arg = immediate(m);
    compare( m, m->a, arg );
    sprintf( disasm, "CMP #$%02X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dex_imp ( gem_mos *m ) { 
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    decrement( m, &m->x );
    sprintf(disasm, "DEX");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cpy_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->y, arg );
    sprintf(disasm, "CPY $%04X ; 0x%02X", addr, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_abs ( gem_mos *m ) {
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf(disasm, "CMP $%04X ; 0x%02X", addr, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dec_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = absolute( m );
    arg = gem_mos_mem_read( m, addr );
    decrement( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "DEC $%04X ; 0x%02X", addr, arg+1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_bne_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, !gem_mos_sr_test( m, SR_FLAG_ZERO ) );
    sprintf(disasm, "BNE $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = indirect_y(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP ($%02X, X) ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x( m );
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP $%02X, X ; 0x%02X", addr-m->x, arg ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dec_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = zeropage_x( m );
    arg = gem_mos_mem_read( m, addr );
    decrement( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "DEC $%02X, X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cld_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_DECIMAL ); 
    sprintf(disasm, "CLD");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_y( m );
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP $%04X, Y ; 0x%04X", addr-m->y, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cmp_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_x( m );
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->a, arg );
    sprintf( disasm, "CMP $%04X, X ; 0x%04X", addr-m->x, arg);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dec_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;
    addr = absolute_x( m );
    arg = gem_mos_mem_read( m, addr );
    decrement( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "DEC $%04X, X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cpx_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    ticks = 2;
    arg = immediate(m);
    compare( m, m->x, arg );
    sprintf( disasm, "CPX #$%02X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = indirect_x( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC ($%02X, X) ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cpx_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 3;
    addr = zeropage(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->x, arg );
    sprintf( disasm, "CPX $%02X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inc_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 5;
    addr = zeropage( m );
    arg = gem_mos_mem_read( m, addr );
    increment( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "INC $%02X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inx_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    increment( m, &m->x );
    sprintf(disasm, "INX");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    arg = immediate(m);
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC #$%02X", arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_nop_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;  
    sprintf(disasm, "NOP");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cpx_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute(m);
    arg = gem_mos_mem_read( m, addr );
    compare( m, m->x, arg );
    sprintf( disasm, "CPX $%04X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC $%04X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inc_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = absolute( m );
    arg = gem_mos_mem_read( m, addr );
    increment( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "INC $%04X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_beq_rel ( gem_mos *m ) {
    GEM_OFFSET offset;
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    offset = relative(m);
    branch( m, offset, gem_mos_sr_test( m, SR_FLAG_ZERO ) );
    sprintf(disasm, "BEQ $%02X", offset&0xFF ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_iny ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = indirect_y( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC ($%02X), Y ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = zeropage_x( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC $%02X, X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inc_zpx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 6;
    addr = zeropage_x( m );
    arg = gem_mos_mem_read( m, addr );
    increment( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "INC $%02X, X ; 0x%02X", addr, arg-1);
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sed_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_DECIMAL );   
    sprintf(disasm, "SED");
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_aby ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_y( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC $%04X, Y ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sbc_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 4;
    addr = absolute_x( m );
    arg = gem_mos_mem_read( m, addr );
    subtract_with_carry( m, arg );
    sprintf( disasm, "SBC $%04X, X ; 0x%02X", addr, arg );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inc_abx ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE arg;
    ticks = 7;
    addr = absolute_x( m );
    arg = gem_mos_mem_read( m, addr );
    increment( m, &arg );
    gem_mos_mem_write( m, addr, arg );
    sprintf(disasm, "INC $%04X, X ; 0x%02X", addr, arg-1);
    return ticks;
}
