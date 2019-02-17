#include <reg51.h>
#define uint	unsigned int
#define uchar	unsigned char

sbit	lcden	= P2 ^ 2;       /* 启用 下降沿 */
sbit	rs	= P2 ^ 0;       /* 高电平输入数据，低电平输入命令 */
sbit	rw	= P2 ^ 1;       /* 高电平读，低电平写 */
sbit	busy	= P0 ^ 7;

char	i, j, temp, num, num_1;
int	a, b, c, result;
int already_clear; //为0表示已经清空，为1表示有结果保存
int mod;
int b_input;
/* a为输入的第一个数，b为输入的第二个数，c为得数 */
/* clear表示是否进行清零操作，如果没有做清零操作，则计算之后再直接输入符号的话，自动把当前计算结果设为第一计算数 */
uchar flag, fuhao;
/*
 * flag=1表示运算符键按下，flag=0表示运算符键没有按下；
 * fuhao=1为加法，fuhao=2为减法，fuhao=3为乘法，fuhao=4为除法。
 */

uchar code	table[] = {                             /* 运算数字输入数组 */
	7, 8, 9, 0,
	4, 5, 6, 0,
	1, 2, 3, 0,
	0, 0, 0, 0
};
uchar code	table1[] = {                            /* 经处理后进行键输入显示准备的数组 */
	7,	     8, 9,	     0x2f - 0x30,       /* 7，8，9，÷ */
	4,	     5, 6,	     0x2a - 0x30,       /* 4, 5, 6，× */
	1,	     2, 3,	     0x2d - 0x30,       /* 1, 2, 3，－ */
	0x01 - 0x30, 0, 0x3d - 0x30, 0x2b - 0x30        /* C，0，=，＋ */
};

void delay( uchar z )                                   /* 延迟函数 */
{
	uchar y;
	for ( z; z > 0; z-- )
		for ( y = 0; y < 110; y++ )
			;
}


void write_com( uchar com )                             /* 写指令函数 */
{
	rs			= 0;
	P0			= com;                  /* com指令付给P0口 */
	delay( 5 ); lcden	= 1; delay( 5 ); lcden = 0;
}


void write_date( uchar date )                           /* 写数据函数 */
{
	rs	= 1; P0 = date; delay( 5 );
	lcden	= 1; delay( 5 ); lcden = 0;
}
void clear_all()
{
	result = 0;
	already_clear = 0;
	write_com( 0x01 );
	j = 0;
	a	= 0;                                    /* 第一个参与运算的数 */
	b	= 0;                                    /* 第二个参与运算的数 */
	c	= 0;
	flag	= 0;                                    /* flag表示是否有符号键按下， */
	fuhao	= 0;  
	b_input = 0;
}


void show_result()
{
	int res = result;
	int num_temp[10];
	int count = 0;
	int ii;
	write_com(0x01);//清屏
	a = result;
	b = 0;
	if(res<0) {write_date(0x3d);//输出一个‘-’
	res = -res;}
	while( res != 0 )
	{
		num_temp[count] = res % 10;
		res /= 10;
		count ++ ;
	}
	for( ii = count - 1 ; ii >= 0 ; ii--)
	{
		write_date( 0x30 + num_temp[ii] );
	}
	already_clear = 0;
}

void show_error()
{
	write_com( 0x80 + 0x4f );    /*按下等于键，光标前进至第二行最后一个显示处 */
	write_com( 0x04 );           /* 设置从后住前写数据，每写完一个数据，光标后退一格 */
	write_date( '!' ); write_date( 'R' ); write_date( 'O' );
	write_date( 'R' ); write_date( 'R' ); write_date( 'E' );
	delay(1000);
	clear_all();
}

void get_calculate()
{
	j = 1;                               /*按下等于键，根据运算符号进行不同的算术处理 */
   if ( fuhao == 1 )                    /* 加法运算 */
   {
	   write_com( 0x80 + 0x4f );    /*按下等于键，光标前进至第二行最后一个显示处 */
	   write_com( 0x04 );           /* 设置从后住前写数据，每写完一个数据，光标后退一格 */
	   c = a + b;
	   result = c;
	   already_clear = 1;
	   if( c == 0 )
		{
		write_date(0x30);
		}
	   while ( c != 0 )
	   {
		   write_date( 0x30 + c % 10 );        c = c / 10;
	   }
	   write_date( 0x3d );          /* 再写"=" */
	   a = 0; b = 0; flag = 0; fuhao = 0;
	   if(result>32767)
	   {
	   	show_error();
	   }
   }  else if ( fuhao == 2 )            /* 减法运算 */
   {
	   write_com( 0x80 + 0x4f );    /* 光标前进至第二行最后一个显示处 */
	   write_com( 0x04 );           /* 设置从后住前写数据，每写完一个数据，光标后退一格(这个照理说顺序不对，可显示和上段一样) */
	   if ( a - b > 0 )
		   c = a - b;
	   else c = b - a;
	   result	= a-b;
	   already_clear = 1;
	   if( c == 0 )
		{
		write_date(0x30);
		}
	   while ( c != 0 )
	   {
		   write_date( 0x30 + c % 10 ); c = c / 10;
	   }
	   if ( a - b < 0 )
		   write_date( 0x2d );
	   write_date( 0x3d );  /* 再写"=" */
	   a = 0; b = 0; flag = 0; fuhao = 0;
	   if(result>32767)
	   {
	   	show_error();
	   }
   }   else if ( fuhao == 3 )   /* 乘法运算 */
   {
	   write_com( 0x80 + 0x4f );            write_com( 0x04 );
	   c		= a * b;
	   result	= c;
	   already_clear = 1;
	   if( c == 0 )
		{
		write_date(0x30);
		}
	   while ( c != 0 )
	   {
		   write_date( 0x30 + c % 10 );        c = c / 10;
	   }
	   write_date( 0x3d );           a = 0; b = 0; flag = 0; fuhao = 0;
	   if(result>32767)
	   {
	   	show_error();
	   }
   }  else if ( fuhao == 4 ) /* 除法运算 */
   {
	   write_com( 0x80 + 0x4f );
	   write_com( 0x04 );
	   i = 0;
	   if ( b != 0 )
	   {
		   c = a / b;
		   result = c;
		   already_clear = 1;
		   mod = a - (c * b);
		   if( c == 0 )
		   {
		   	write_date(0x30);
		   }
		   while ( c != 0 )
		   {
		   		write_date( 0x30 + c % 10 );        c = c / 10;					   	
		   }
		   write_date( 0x3d );
		    write_com( 0x80 + 0x23);
	   		write_com( 0x04 );
	   		if(mod == 0)
	   			write_date( 0x30 );
	   		while(mod != 0)
	   		{
	   			write_date( 0x30 + mod % 10 );
	   			mod = mod / 10;
	   		}
	   		write_date(0x3d);
	   		write_date(0x64);
	   		write_date(0x6f);
	   		write_date(0x6d);
		   a = 0; b = 0; flag = 0; fuhao = 0;
	   }else  {
		   write_date( '!' ); write_date( 'R' ); write_date( 'O' );
		   write_date( 'R' ); write_date( 'R' ); write_date( 'E' );
	   }
   }
}

void init()                                             /* 初始化 */
{
	write_com( 0x01 );
	num	= -1;
	lcden	= 1;                                    /* 使能信号为高电平 */
	rw	= 0;
	write_com( 0x38 );                              /* 8位，2行 */
	delay( 5 ); write_com( 0x38 );                  /* 8位，2行 */
	delay( 5 ); write_com( 0x0c );                  /* 显示开，光标关，不闪烁* / */
	delay( 1 ); write_com( 0x06 );                  /* 增量方式不移位 */
	delay( 1 ); write_com( 0x80 );                  /* 检测忙信号 */
	delay( 1 ); write_com( 0x01 );                  /* 显示开，光标关，不闪烁 */
	num_1	= 0;
	i	= 0; j = 0;
	a	= 0;                                    /* 第一个参与运算的数 */
	b	= 0;                                    /* 第二个参与运算的数 */
	c	= 0;
	flag	= 0;                                    /* flag表示是否有符号键按下， */
	fuhao	= 0;                                    /* fuhao表征按下的是哪个符号 */
	already_clear = 0;
	b_input = 0;
}


/*
 * 对于4×4键盘需要8条数据线，4根行线，4根列线。都接在P3口上，低4位接行4根线，高4位接列4根线。如何确定16个键那个被按下，是通过程序对键盘扫描，过程是这样的，先让第一行线为低电平代码为11111110转换成十六进制为0xfe，紧接着下条语句是读取P3口值并于0xfe比较，如果相等说明没有按键被安下，如果不相等有按键被按下，假如第1行第1列按键被按下这时读取值为11101110，高4位是表示列数，高4位1110表示第一列上的按键有被按下的，再通过低4位值1110可判定第一行第一列的按键被按。
 * 这样完成第一行的扫描，紧着要进行第二行的扫描，使P3等于11111101十六进制为0xfd。再完成上述过程。再往下是第三，第四行。以上过程不停的循环。
 */

void keyscan()
{
	P3 = 0xfe;                                      /* 让第一行线是低电平 */
	if ( P3 != 0xfe )
	{
		delay( 20 );                            /* 延迟20ms */
		if ( P3 != 0xfe )
		{
			temp = P3 & 0xf0;
			switch ( temp )
			{
			case 0xe0: num	= 0;   break;   /* 7 */
			case 0xd0: num	= 1;   break;   /* 8 */
			case 0xb0: num	= 2;   break;   /* 9 */
			case 0x70: num	= 3;   break;   /* ÷ */
			}
		}
		while ( P3 != 0xfe )
			;
		if ( num == 0 || num == 1 || num == 2 ) /* 如果按下的是'7','8'或'9 */
		{
			if ( j != 0 )
			{
				clear_all();
			}
			if ( flag == 0 )                /* 没有按过运算符键 */
			{
				a = a * 10 + table[num];
			}                               /*按下数字存储到a */
			else{ /* 如果按过运算符键 */
				b = b * 10 + table[num];
				b_input = 1;
			}                               /*按下数字存储到b */
		}else  { /* 如果按下的是'/'            除法 */
			flag += 1;                    /*按下运算符 */
			if(already_clear == 1)
			{
				show_result();
				already_clear = 0;
				flag = 1;
				j = 0;
			}
			else if(flag > 1)
			{
				get_calculate();
				show_result();
				flag = 1;
				j = 0;
			}
			fuhao = 4;                    /* 4表示除号已按 */
		}
		i = table1[num];                        /* 数据显示做准备 */
		write_date( 0x30 + i );                 /* 显示数据或操作符号 */
	}


	P3 = 0xfd;
	if ( P3 != 0xfd )
	{
		delay( 20 );
		if ( P3 != 0xfd )
		{
			temp = P3 & 0xf0;
			switch ( temp )
			{
			case 0xe0: num	= 4; break;                     /* 4 */
			case 0xd0: num	= 5; break;                     /* 5 */
			case 0xb0: num	= 6; break;                     /* 6 */
			case 0x70: num	= 7; break;                     /* × */
			}
		}
		while ( P3 != 0xfd )
			;                                               /* 等待按键释放 */
		if ( num == 4 || num == 5 || num == 6 && num != 7 )     /* 如果按下的是'4','5'或'6' */
		{
			if ( j != 0 )
			{
				clear_all();
			}
			if ( flag == 0 )                                /* 没有按过运算符键 */
			{
				a = a * 10 + table[num];
			}else                           { /* 如果按过运算符键 */
				b = b * 10 + table[num];
				b_input = 1;
			}
		}else  { /* 如果按下的是'×' */
			flag += 1;                    /*按下运算符 */
			if(already_clear == 1)
			{
				show_result();
				already_clear = 0;
				flag = 1;
				j = 0;
			}
			else if(flag > 1)
			{
				get_calculate();
				show_result();
				flag = 1;
				j = 0;
			}
			fuhao	= 3;    /* 3表示乘号已按 */
		}
		i = table1[num];        /* 数据显示做准备 */
		write_date( 0x30 + i ); /* 显示数据或操作符号 */
	}

	P3 = 0xfb;
	if ( P3 != 0xfb )
	{
		delay( 20 );
		if ( P3 != 0xfb )
		{
			temp = P3 & 0xf0;
			switch ( temp )
			{
			case 0xe0: num	= 8;    break;          /* 1 */
			case 0xd0: num	= 9;    break;          /* 2 */
			case 0xb0: num	= 10;   break;          /* 3 */
			case 0x70: num	= 11;   break;          /* - */
			}
		}
		while ( P3 != 0xfb )
			;
		if ( num == 8 || num == 9 || num == 10 )        /* 如果按下的是'1','2'或'3' */
		{
			if ( j != 0 )
			{
				clear_all();
			}
			if ( flag == 0 )                        /* 没有按过运算符键 */
			{
				a = a * 10 + table[num];
			}else                         { /* 如果按过运算符键 */
				b = b * 10 + table[num];
				b_input = 1;
			}
		}else if ( num == 11 )                          /* 如果按下的是'-' */
		{
			flag += 1;                    /*按下运算符 */
			if(already_clear == 1)
			{
				show_result();
				already_clear = 0;
				flag = 1;
				j = 0;
			}
			else if(flag > 1)
			{
				get_calculate();
				show_result();
				flag = 1;
				j = 0;
			}
			fuhao	= 2;                            /* 2表示减号已按 */
		}
		i = table1[num];                                /* 数据显示做准备 */
		write_date( 0x30 + i );                         /* 显示数据或操作符号 */
	}




	P3 = 0xf7;
	if ( P3 != 0xf7 )
	{
		delay( 20 );
		if ( P3 != 0xf7 )
		{
			temp = P3 & 0xf0;
			switch ( temp )
			{
			case 0xe0: num	= 12; break;    /* 清0键 */
			case 0xd0: num	= 13; break;    /* 数字0 */
			case 0xb0: num	= 14; break;    /* 等于键 */
			case 0x70: num	= 15; break;    /* 加 */
			}
		}
		while ( P3 != 0xf7 )
			;
	if( num == 12 )
	{
		clear_all();
	}
	if( num == 13 )
	{
		if ( flag == 0 ) /* 没有按过运算符键 */
		{
			a = a * 10;  write_date( 0x30 );     P3 = 0;
		}else if ( flag >= 1 ) /* 如果按过运算符键 */
		{
			b = b * 10;   write_date( 0x30 );
			b_input = 1;
		}
	}
	if( num == 14 )
	{
		get_calculate();
	}
	if( num == 15 )
	{
		/*if(b_input == 0)
			{
				write_com( 0x04 );
				write_date( 0x20 );
			}*/
		 	flag += 1;                    /*按下运算符+ */
			if(already_clear == 1)
			{
				show_result();
				already_clear = 0;
				flag = 1;
				j = 0;
			}
			else if(flag > 1)
			{
				get_calculate();
				show_result();
				flag = 1;
				j = 0;
			} 
		 fuhao = 1; 
		i = table1[num];                                /* 数据显示做准备 */
		write_date( 0x30 + i );                         /* 显示数据或操作符号 */
		 //write_com( 0x05 );
	}

	}
}

main()
{
	init();
	while ( 1 )
	{
		keyscan();
	}
}

