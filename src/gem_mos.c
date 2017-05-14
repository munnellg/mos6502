#include "gem_mos.h"

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
    NULL, /* 0x00 */
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

    NULL, /* 0x10 */
    NULL, /* 0x11 */
    NULL, /* 0x12 */
    NULL, /* 0x13 */
    NULL, /* 0x14 */
    NULL, /* 0x15 */
    NULL, /* 0x16 */
    NULL, /* 0x17 */
    gem_mos_clc_imp, /* 0x18 */
    NULL, /* 0x19 */
    NULL, /* 0x1A */
    NULL, /* 0x1B */
    NULL, /* 0x1C */
    NULL, /* 0x1D */
    NULL, /* 0x1E */
    NULL, /* 0x1F */

    NULL, /* 0x20 */
    NULL, /* 0x21 */
    NULL, /* 0x22 */
    NULL, /* 0x23 */
    NULL, /* 0x24 */
    NULL, /* 0x25 */
    NULL, /* 0x26 */
    NULL, /* 0x27 */
    gem_mos_plp_imp, /* 0x28 */
    NULL, /* 0x29 */
    NULL, /* 0x2A */
    NULL, /* 0x2B */
    NULL, /* 0x2C */
    NULL, /* 0x2D */
    NULL, /* 0x2E */
    NULL, /* 0x2F */

    NULL, /* 0x30 */
    NULL, /* 0x31 */
    NULL, /* 0x32 */
    NULL, /* 0x33 */
    NULL, /* 0x34 */
    NULL, /* 0x35 */
    NULL, /* 0x36 */
    NULL, /* 0x37 */
    gem_mos_sec_imp, /* 0x38 */
    NULL, /* 0x39 */
    NULL, /* 0x3A */
    NULL, /* 0x3B */
    NULL, /* 0x3C */
    NULL, /* 0x3D */
    NULL, /* 0x3E */
    NULL, /* 0x3F */
    
    NULL, /* 0x40 */
    NULL, /* 0x41 */
    NULL, /* 0x42 */
    NULL, /* 0x43 */
    NULL, /* 0x44 */
    NULL, /* 0x45 */
    NULL, /* 0x46 */
    NULL, /* 0x47 */
    gem_mos_pha_imp, /* 0x48 */
    NULL, /* 0x49 */
    NULL, /* 0x4A */
    NULL, /* 0x4B */
    NULL, /* 0x4C */
    NULL, /* 0x4D */
    NULL, /* 0x4E */
    NULL, /* 0x4F */

    NULL, /* 0x50 */
    NULL, /* 0x51 */
    NULL, /* 0x52 */
    NULL, /* 0x53 */
    NULL, /* 0x54 */
    NULL, /* 0x55 */
    NULL, /* 0x56 */
    NULL, /* 0x57 */
    gem_mos_cli_imp, /* 0x58 */
    NULL, /* 0x59 */
    NULL, /* 0x5A */
    NULL, /* 0x5B */
    NULL, /* 0x5C */
    NULL, /* 0x5D */
    NULL, /* 0x5E */
    NULL, /* 0x5F */

    NULL, /* 0x60 */
    NULL, /* 0x61 */
    NULL, /* 0x62 */
    NULL, /* 0x63 */
    NULL, /* 0x64 */
    NULL, /* 0x65 */
    NULL, /* 0x66 */
    NULL, /* 0x67 */
    gem_mos_pla_imp, /* 0x68 */
    NULL, /* 0x69 */
    NULL, /* 0x6A */
    NULL, /* 0x6B */
    NULL, /* 0x6C */
    NULL, /* 0x6D */
    NULL, /* 0x6E */
    NULL, /* 0x6F */

    NULL, /* 0x70 */
    NULL, /* 0x71 */
    NULL, /* 0x72 */
    NULL, /* 0x73 */
    NULL, /* 0x74 */
    NULL, /* 0x75 */
    NULL, /* 0x76 */
    NULL, /* 0x77 */
    gem_mos_sei_imp, /* 0x78 */
    NULL, /* 0x79 */
    NULL, /* 0x7A */
    NULL, /* 0x7B */
    NULL, /* 0x7C */
    NULL, /* 0x7D */
    NULL, /* 0x7E */
    NULL, /* 0x7F */

    NULL, /* 0x80 */
    NULL, /* 0x81 */
    NULL, /* 0x82 */
    NULL, /* 0x83 */
    NULL, /* 0x84 */
    NULL, /* 0x85 */
    NULL, /* 0x86 */
    NULL, /* 0x87 */
    gem_mos_dey_imp, /* 0x88 */
    NULL, /* 0x89 */
    gem_mos_txa_imp, /* 0x8A */
    NULL, /* 0x8B */
    NULL, /* 0x8C */
    gem_mos_sta_abs, /* 0x8D */
    NULL, /* 0x8E */
    NULL, /* 0x8F */

    NULL, /* 0x90 */
    NULL, /* 0x91 */
    NULL, /* 0x92 */
    NULL, /* 0x93 */
    NULL, /* 0x94 */
    NULL, /* 0x95 */
    NULL, /* 0x96 */
    NULL, /* 0x97 */
    gem_mos_tya_imp, /* 0x98 */
    NULL, /* 0x99 */
    gem_mos_txs_imp, /* 0x9A */
    NULL, /* 0x9B */
    NULL, /* 0x9C */
    NULL, /* 0x9D */
    NULL, /* 0x9E */
    NULL, /* 0x9F */

    gem_mos_ldy_imm, /* 0xA0 */
    NULL, /* 0xA1 */
    gem_mos_ldx_imm, /* 0xA2 */
    NULL, /* 0xA3 */
    NULL, /* 0xA4 */
    NULL, /* 0xA5 */
    NULL, /* 0xA6 */
    NULL, /* 0xA7 */
    gem_mos_tay_imp, /* 0xA8 */
    gem_mos_lda_imm, /* 0xA9 */
    gem_mos_tax_imp, /* 0xAA */
    NULL, /* 0xAB */
    NULL, /* 0xAC */
    NULL, /* 0xAD */
    NULL, /* 0xAE */
    NULL, /* 0xAF */

    NULL, /* 0xB0 */
    NULL, /* 0xB1 */
    NULL, /* 0xB2 */
    NULL, /* 0xB3 */
    NULL, /* 0xB4 */
    NULL, /* 0xB5 */
    NULL, /* 0xB6 */
    NULL, /* 0xB7 */
    gem_mos_clv_imp, /* 0xB8 */
    NULL, /* 0xB9 */
    gem_mos_tsx_imp, /* 0xBA */
    NULL, /* 0xBB */
    NULL, /* 0xBC */
    NULL, /* 0xBD */
    NULL, /* 0xBE */
    NULL, /* 0xBF */

    NULL, /* 0xC0 */
    NULL, /* 0xC1 */
    NULL, /* 0xC2 */
    NULL, /* 0xC3 */
    NULL, /* 0xC4 */
    NULL, /* 0xC5 */
    NULL, /* 0xC6 */
    NULL, /* 0xC7 */
    gem_mos_iny_imp, /* 0xC8 */
    NULL, /* 0xC9 */
    gem_mos_dex_imp, /* 0xCA */
    NULL, /* 0xCB */
    NULL, /* 0xCC */
    NULL, /* 0xCD */
    NULL, /* 0xCE */
    NULL, /* 0xCF */

    NULL, /* 0xD0 */
    NULL, /* 0xD1 */
    NULL, /* 0xD2 */
    NULL, /* 0xD3 */
    NULL, /* 0xD4 */
    NULL, /* 0xD5 */
    NULL, /* 0xD6 */
    NULL, /* 0xD7 */
    gem_mos_cld_imp, /* 0xD8 */
    NULL, /* 0xD9 */
    NULL, /* 0xDA */
    NULL, /* 0xDB */
    NULL, /* 0xDC */
    NULL, /* 0xDD */
    NULL, /* 0xDE */
    NULL, /* 0xDF */

    NULL, /* 0xE0 */
    NULL, /* 0xE1 */
    NULL, /* 0xE2 */
    NULL, /* 0xE3 */
    NULL, /* 0xE4 */
    NULL, /* 0xE5 */
    NULL, /* 0xE6 */
    NULL, /* 0xE7 */
    gem_mos_inx_imp, /* 0xE8 */
    NULL, /* 0xE9 */
    gem_mos_nop_imp, /* 0xEA */
    NULL, /* 0xEB */
    NULL, /* 0xEC */
    NULL, /* 0xED */
    NULL, /* 0xEE */
    NULL, /* 0xEF */

    NULL, /* 0xF0 */
    NULL, /* 0xF1 */
    NULL, /* 0xF2 */
    NULL, /* 0xF3 */
    NULL, /* 0xF4 */
    NULL, /* 0xF5 */
    NULL, /* 0xF6 */
    NULL, /* 0xF7 */
    NULL, /* 0xF8 */
    NULL, /* 0xF9 */
    NULL, /* 0xFA */
    NULL, /* 0xFB */
    NULL, /* 0xFC */
    NULL, /* 0xFD */
    gem_mos_sed_imp, /* 0xFE */
    NULL, /* 0xFF */
};

/*---------------------------------------------------------------------------
 *
 *  Addressing modes for the MOS 6502. These affect how the arguments to a
 *  given instruction are retrieved. With the exception of the immediate mode
 *  (which actually returns the argument itself), these functions all return
 *  the memory address of the argument so that it may be fetched by the 
 *  instruction.
 *
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
    addr = gem_mos_mem_readw( m, m->pc );
    m->pc += 2;
    return addr;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR
absolute_x ( gem_mos *m ) {
    /* TODO: Investigate the add without carry for this addressing mode */
    GEM_MEMORY_ADDR addr;    
    addr = absolute(m);
    return addr + m->x;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR
absolute_y ( gem_mos *m ) {
    /* TODO: Investigate the add with carry for this addressing mode */
    GEM_MEMORY_ADDR addr;    
    addr = absolute(m);
    return addr + m->y;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_BYTE
immediate ( gem_mos *m ) {
    GEM_MEMORY_BYTE val;    
    val = gem_mos_mem_read( m, m->pc++ );
    return val;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
 * ============================================================================
 */
static GEM_MEMORY_ADDR
indirect ( gem_mos *m ) {
    /* TODO: Check for page crossings */
    GEM_MEMORY_ADDR addr, eaddr;
    addr = absolute(m); 
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
    addr = gem_mos_mem_read( m, m->pc++ );  
    eaddr = gem_mos_mem_readw( m, addr );
    return eaddr + m->y;
}

/*
 * ===  FUNCTION  =============================================================
 *          Name: 
 *  Desecription: 
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

static void
arithmetic_shift_left ( gem_mos *m, GEM_REGISTER8 *a ) {
    gem_mos_sr_update( m, SR_FLAG_CARRY, GEM_TEST_MSB(*a) );
    *a <<= 1;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
logical_or ( gem_mos *m, GEM_REGISTER8 *a, GEM_REGISTER8 b ) {
    *a |= b;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*a) );
}

static void
transfer ( gem_mos *m, GEM_REGISTER8 *a, GEM_REGISTER8 *b ) {
    *b = *a;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(*b) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(*b) );
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
    return m->sr & flag;
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
    FILE *f;
    int32_t nbytes;

    GEM_MEMORY_ADDR addr;

    f = fopen(fname, "r");

    if(!f) {
        return -1;
    }
    
    addr = GEM_ROM_ADDR;
    /* TODO: Limit this so that it only reads into valid ROM memory. I'm not
     * sure how big that will be, but right now this is problematic */  
    while(!feof(f)) {
        m->memory[addr++] = getc(f);
    }

    nbytes = ftell(f);

    fclose(f);

    return nbytes; 
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
    /* TODO: Fail gracefully for invalid opcode. Right now I feel it is the
     * responsibility of the developer to make sure invalid code isn't run,
     * but maybe the library should do something to prevent catastropic 
     * failure */
    return jump_table[code](m); 
}

/* fetch and execute next CPU instruction */
void
gem_mos_step ( gem_mos *m ) {
    GEM_OPCODE code;

    code = gem_mos_fetch(m, m->pc++);
    m->clock += gem_mos_execute( m, code );
}

void
gem_mos_free ( gem_mos *m ) {
    if(m) {
        free(m);
    }
}

/* TODO: Devise a nicer way to compute ticks for each instruction */

GEM_CLOCK_TICKS
gem_mos_ora_xin ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    ticks = 3;          
    logical_or(m, &m->a, gem_mos_mem_read( m, indirect_x(m) ));
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;  
    ticks = 3;      
    logical_or(m, &m->a, gem_mos_mem_read( m, zeropage(m) ));   
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_zpg ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE temp;
    ticks = 5;      
    addr = zeropage(m);
    temp = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &temp );  
    gem_mos_mem_write( m, addr, temp );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_php_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 3;
    gem_mos_sp_push( m, m->sr );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 3;
    logical_or(m, &m->a, immediate(m) );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_acc ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;          
    arithmetic_shift_left( m, &m->a );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ora_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    logical_or(m, &m->a, gem_mos_mem_read( m, absolute(m) ) );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_asl_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    GEM_MEMORY_ADDR addr;
    GEM_MEMORY_BYTE temp;
    ticks = 6;      
    addr = absolute(m);
    temp = gem_mos_mem_read( m, addr );
    arithmetic_shift_left( m, &temp );  
    gem_mos_mem_write( m, addr, temp );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_clc_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_CARRY );   
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_plp_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    m->sr = gem_mos_sp_pop( m );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sec_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_CARRY ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_pha_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 3;
    gem_mos_sp_push( m, m->a ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cli_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_INTERRUPT );   
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_pla_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 4;
    m->a = gem_mos_sp_pop( m ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sei_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_INTERRUPT ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dey_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->y--;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->y) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->y) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tya_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->y, &m->a );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldy_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->y = immediate(m);
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->y) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->y) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_ldx_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->x = immediate(m);
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->x) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_lda_imm ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->a = immediate(m);
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->a) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->a) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tay_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->a, &m->y );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_clv_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_OVERFLOW );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_iny_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->y++;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->y) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->y) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_cld_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_unset( m, SR_FLAG_DECIMAL ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_inx_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->x++;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->x) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sed_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    gem_mos_sr_set( m, SR_FLAG_DECIMAL );   
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_txa_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->x, &m->a );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_sta_abs ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;    
    ticks = 4;    
    gem_mos_mem_write( m, absolute(m), m->a );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_txs_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->x, &m->sp );
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tax_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->a, &m->x ); 
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_tsx_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    transfer(m, &m->sp, &m->x );    
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_dex_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;
    m->x--;
    gem_mos_sr_update( m, SR_FLAG_ZERO, GEM_TEST_ZERO(m->x) );
    gem_mos_sr_update( m, SR_FLAG_NEGATIVE, GEM_TEST_NEGATIVE(m->x) );  
    return ticks;
}

GEM_CLOCK_TICKS
gem_mos_nop_imp ( gem_mos *m ) {
    GEM_CLOCK_TICKS ticks;
    ticks = 2;  
    return ticks;
}
