#ifndef  _TYPEDEF_H
  #define  _TYPEDEF_H

//--------   Стандартные типы данных   -----------------------
typedef   unsigned char       byte  ;
typedef   unsigned short      word  ;
typedef   unsigned long       lword ;

typedef   signed char         sbyte  ;
typedef   signed short        sword  ;
typedef   signed long         slword ;

typedef   signed              s  ;
typedef   unsigned            u  ;
typedef   float               f  ;
typedef   double              d  ;

typedef   signed char         int8  ;
typedef   signed short        int16 ;
typedef   signed int          int32 ;
typedef   signed long long    int64 ;

typedef   unsigned char       uint8  ;
typedef   unsigned short      uint16 ;
typedef   unsigned int        uint32 ;
typedef   unsigned long long  uint64 ;

typedef   unsigned char       b  ;
typedef   unsigned short      w  ;
typedef   unsigned long       lw ;

typedef   signed char         sb  ;
typedef   signed short        sw  ;
typedef   signed long         slw ;


//------------------------------------------------------------






#define asm_di()  __disable_interrupt()
#define asm_ei()  __enable_interrupt()

#endif

