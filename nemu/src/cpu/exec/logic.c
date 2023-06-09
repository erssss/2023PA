#include "cpu/exec.h"

make_EHelper(test) {
  // TODO();
  rtl_and(&t2,&id_dest->val,&id_src->val);
  rtl_update_ZFSF(&t2,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(test);
}

make_EHelper(and) {
  // TODO();
  rtl_and(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  // TODO();
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // TODO();
  rtl_or(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // TODO();
  rtl_sext(&t2,&id_dest->val,id_dest->width);
  rtl_sar(&t2,&t2,&id_src->val);
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  // TODO();
  rtl_shl(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  // TODO();
  rtl_shr(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  // TODO();
  rtl_not(&id_dest->val);
  operand_write(id_dest,&id_dest->val);

  print_asm_template1(not);
}

make_EHelper(rol) {
	  uint32_t flag1,flag2; 
	  rtl_mv(&flag1,&id_src->val);
    while(flag1!=0) {
		  rtl_msb(&flag2,&id_dest->val,id_dest->width);
		  rtl_shli(&id_dest->val,&id_dest->val,1);
		  rtl_add(&id_dest->val,&id_dest->val,&flag2);
		  flag1--;
	  }  
    
	  operand_write(id_dest,&id_dest->val);
	  rtl_andi(&t0,&id_dest->val,0x1);
	  rtl_set_CF(&t0);
    
	  if(id_src->val==1) {
		  rtl_get_CF(&t0);
		  rtl_msb(&flag2,&id_dest->val,id_dest->width);
		  if(t0!=flag2) {							
			  rtl_li(&t1,1);					
			  rtl_set_OF(&t1);					  
		  }  
      else {
        rtl_set_OF(&tzero);
      }
    }
    print_asm_template1(rol);
}

make_EHelper(shld)
{
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  rtl_li(&t2, id_src2->width);
  rtl_shli(&t2, &t2, 3);
  rtl_subi(&t2, &t2, id_src->val);
  rtl_shr(&t2, &id_src2->val, &t2);
  rtl_or(&t0, &t0, &t2);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  print_asm_template3(shld);
}

make_EHelper(shrd)
{
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  rtl_li(&t2, id_src2->width);
  rtl_shli(&t2, &t2, 3);
  rtl_subi(&t2, &t2, id_src->val);
  rtl_shl(&t2, &id_src2->val, &t2);
  rtl_or(&t0, &t0, &t2);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);
  print_asm_template3(shrd);
} 