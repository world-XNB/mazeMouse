#include "main.h"

bit irC=0,irL=0,irR=0,irLU=0,irRU=0;//定义红外传感器检测状态全局位变量，为0无障碍
unsigned char counter_IR=1;
unsigned char code table[] = {0xc0,0xf9,0xa4,0xb0,0x99, 0x92,0x82,0xf8,0x80,0x90};//数码管用来显示坐标
unsigned char code forword[] = {0x11,0x93,0x82,0xc6,0x44,0x6c,0x28,0x39};//直走数组
unsigned char code positive[] = {0x11,0x33,0x22,0x66,0x44,0xcc,0x88,0x99};//顺时针转-右转
unsigned char code counter[] = {0x11,0x99,0x88,0xcc,0x44,0x66,0x22,0x33};//逆时针转-左转
unsigned char l_absolute_direction=3,c_absolute_direction=3,relative_direction=0;//绝对方向和相对方向
//*******************************************************************准备部分：栈，队列***********************************************************
//maze
unsigned char maze[8][8];//地图数组
unsigned char step[8][8];//步数数组
unsigned char mouse_x=0,mouse_y=0;//迷宫鼠坐标
unsigned char count_step=0;       //步数
void MOUSE_IR_ON(GROUP_NO) {//开红外函数
	do{ 
		A0=(GROUP_NO)&0x01;
		A1=(GROUP_NO)&0x02; 
		A2=(GROUP_NO)&0x04;
		}while(0);
}
//stack
unsigned char  stack_x[40];   //建立栈
unsigned char  stack_y[40];   //建立栈
unsigned char  stack_top = 0; //栈顶
void push(unsigned char x,unsigned char y){//入队
		stack_top++;
		stack_x[stack_top] = x;  
		stack_y[stack_top] = y;  
}
void pop(){//出队
	stack_top--;
}
//queue
typedef struct{//坐标结构体
    unsigned char x;
    unsigned char y;
}coordinate;
coordinate queue[10];  //队列数组
coordinate move[4];    //方向数组
unsigned char front=0,rear=0;      //队头和队尾
void queue_in(unsigned char in_x,unsigned char in_y){//入栈
    queue[rear].x = in_x;
    queue[rear].y = in_y;
    rear=(rear+1)%10;
}
void queue_out(){//出栈
    front=(front+1)%10;
}
void move_int(){//初始化用来判断一个格子的四个方向
    move[0].x = 1;
    move[0].y = 0;
    move[1].x = 0;
    move[1].y = -1;
    move[2].x = -1;
    move[2].y = 0;
    move[3].x = 0;
    move[3].y = 1;
}
//delay function
void delay_ms(unsigned int z){//延时函数
	unsigned char i,j;
	while(--z){
		_nop_();
		i = 2;
		j = 199;
	}
	do{
		while(--j);
	}while(--i);
}
//Infrared sensor-time2
void  Time2_init(){//定时器初始化函数
	EA=1;
	ET2=1;
	TH2=RCAP2H=(65536-7000)/256; 
	TL2=RCAP2L=(65536-7000)%256;
	TR2=1;
}
//*******************************************************************其他驱动部分*****************************************************************
void beep_on(){//蜂鸣器
	Beep = 0;
	delay_ms(10000);
	Beep = 1;
	delay_ms(10000);
}
void tube1_on(unsigned char x){//数码管1
	tube1 = 0;
	tube2 = 1;
	P0 = table[x];
}
void tube2_on(unsigned char x){//数码管2
	tube1 = 1;
	tube2 = 0;
	P0 = table[x];
}

//*******************************************************************电机驱动部分*****************************************************************
void gostright(){//遍历时的直走函数，包含记录信息
	unsigned char i,j;
	delay_ms(1500);//走一步延时
	for(j=0;j<104;j++){	//j用来调距离
			for(i=0;i<8;i++){
				P1=(forword[i]);
				delay_ms(80);
			}
		}
	update();
	maze[mouse_x][mouse_y] &= note_current();//记录迷宫墙的信息
	maze[mouse_x][mouse_y] &= note_current();//记录迷宫墙的信息
	note_direction();//记录来的路径
	delay_ms(1500);//走一步延时
}
void backstright(){//回溯时直走函数，只直走不记录信息
	unsigned char i,j;
	for(j=0;j<104;j++){	//j用来调距离
			for(i=0;i<8;i++){
				P1=(forword[i]);
				delay_ms(80);
			}
		}
	update();
	delay_ms(1500);//走一步延时
}
void turnleft(){ //左转 
	unsigned char i,j;	//j用来调距离
	delay_ms(1000);//走一步延时
	TR2=0;
	for(j=0;j<50;j++){
		for(i=0;i<8;i++){
			P1=counter[i];
			delay_ms(20);
		}
	}
	c_absolute_direction = (l_absolute_direction+3)%4;//相对方向到绝对方向的转换
	l_absolute_direction = c_absolute_direction;
	TR2=1;
	delay_ms(1000);//走一步延时
}
void turnright(){//右转 
	unsigned char i,j;	//j用来调距离
	delay_ms(1000);//走一步延时
	TR2=0;
	for(j=0;j<50;j++){
		for(i=0;i<8;i++){
			P1=positive[i];
			delay_ms(20);
		}
	}
	c_absolute_direction = (l_absolute_direction+1)%4;//相对方向到绝对方向的转换
	l_absolute_direction = c_absolute_direction;
	TR2=1;
	delay_ms(1000);//走一步延时
}
//*******************************************************************功能函数部分*****************************************************************
void maze_init(){//地图数组初始化
	unsigned char i,j;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++)
			maze[i][j] = 0xff;
	}
}
void step_init(){//步数函数初始化
	unsigned char i,j;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++)
			step[i][j] = 70;
	}
}
unsigned char note_current(){//当前迷宫格墙的信息->低四位 ：0表示没有挡板，1表示有挡板(高四位此时为1)
	unsigned char current;
	switch(c_absolute_direction){
		case 0:	if(irC==1&&irR==1&&irL==1)//死路
							current = 0x0d;
						else if(irC==0&&irR==1&&irL==1)//前边无障碍
							current = 0x05;
						else if(irC==0&&irR==1&&irL==0)//前边和左边无障碍
							current = 0x04;
						else if(irC==0&&irR==0&&irL==1)//前边和右边无障碍
							current = 0x01;
						else if(irC==1&&irR==0&&irL==1)//右边无障碍
							current = 0x09;
						else if(irC==1&&irR==1&&irL==0)//左边无障碍
							current = 0x0c;
						else if(irC==1&&irR==0&&irL==0)//右边和左边无障碍
							current = 0x08;
						else if(irC==0&&irR==0&&irL==0)//三边无障碍
							current = 0x00;break;
		case 1:	if(irC==1&&irR==1&&irL==1)//死路
							current = 0x0e;
						else if(irC==0&&irR==1&&irL==1)//前边无障碍
							current = 0x0a;
						else if(irC==0&&irR==1&&irL==0)//前边和左边无障碍
							current = 0x02;
						else if(irC==0&&irR==0&&irL==1)//前边和右边无障碍
							current = 0x08;
						else if(irC==1&&irR==0&&irL==1)//右边无障碍
							current = 0x0c;
						else if(irC==1&&irR==1&&irL==0)//左边无障碍
							current = 0x06;
						else if(irC==1&&irR==0&&irL==0)//右边和左边无障碍
							current = 0x04;
						else if(irC==0&&irR==0&&irL==0)//三边无障碍
							current = 0x00;break;
		case 2:	if(irC==1&&irR==1&&irL==1)//死路
							current = 0x07;
						else if(irC==0&&irR==1&&irL==1)//前边无障碍
							current = 0x05;
						else if(irC==0&&irR==1&&irL==0)//前边和左边无障碍
							current = 0x01;
						else if(irC==0&&irR==0&&irL==1)//前边和右边无障碍
							current = 0x04;
						else if(irC==1&&irR==0&&irL==1)//右边无障碍
							current = 0x04;
						else if(irC==1&&irR==1&&irL==0)//左边无障碍
							current = 0x03;
						else if(irC==1&&irR==0&&irL==0)//右边和左边无障碍
							current = 0x02;
						else if(irC==0&&irR==0&&irL==0)//三边无障碍
							current = 0x00;break;
		case 3:	if(irC==1&&irR==1&&irL==1)//死路
							current = 0x0b;
						else if(irC==0&&irR==1&&irL==1)//前边无障碍
							current = 0x0a;
						else if(irC==0&&irR==1&&irL==0)//前边和左边无障碍
							current = 0x08;
						else if(irC==0&&irR==0&&irL==1)//前边和右边无障碍
							current = 0x02;
						else if(irC==1&&irR==0&&irL==1)//右边无障碍
							current = 0x03;
						else if(irC==1&&irR==1&&irL==0)//左边无障碍
							current = 0x09;
						else if(irC==1&&irR==0&&irL==0)//右边和左边无障碍
							current = 0x01;
						else if(irC==0&&irR==0&&irL==0)//三边无障碍
							current = 0x00;break;
					}
	return current;
}
void note_direction(){//记录上一个迷宫格到这个迷宫格的绝对方向 ->高四位：(0表示从这个方向来,1表示不从这个方向来)
		step[mouse_x][mouse_y]=c_absolute_direction;
}
void update(){//坐标的刷新
	if(l_absolute_direction == 0)//迷宫格0
			mouse_x +=1;
	if(l_absolute_direction == 1)//迷宫格1
			mouse_y -=1;
	if(l_absolute_direction == 2)//迷宫格2
			mouse_x -=1;
	if(l_absolute_direction == 3)//迷宫格3
			mouse_y +=1;
}
unsigned char take_direction(){//取进入当前格子时的方向——>方便回溯
	unsigned char direction;//存高四位的信息
	direction=step[mouse_x][mouse_y];
	return direction;
}
unsigned char take_current(unsigned char x,unsigned char y,unsigned char i){//取当前迷宫对应方向是否可以走，没有墙返回1，有墙返回0用于回溯
	unsigned char temp;
	temp = maze[x][y]&0x0f;
	switch(i){
		case 0:if(temp==0x00||temp==0x04||temp==0x02||temp==0x01||temp==0x03||temp==0x06||temp==0x05||temp==0x07)//0
								return 1;
					 else
					    	return 0;
		case 1:if(temp==0x00||temp==0x08||temp==0x02||temp==0x01||temp==0x09||temp==0x03||temp==0x0a||temp==0x0b)//1
								return 1;
					 else
								return 0;
		case 2:if(temp==0x00||temp==0x08||temp==0x04||temp==0x01||temp==0x0c||temp==0x09||temp==0x05||temp==0x0d)//2
								return 1;
						else
								return 0;
		case 3:if(temp==0x00||temp==0x08||temp==0x04||temp==0x02||temp==0x0c||temp==0x06||temp==0x0a||temp==0x0e)//3
								return 1;
						else
								return 0;
	}
}
//****************************************************************算法：遍历部分*******************************************************************
unsigned char go_next(){//走下一步，并返回1和0是否可走
	switch(c_absolute_direction){
		case 0:if(irL==0&&step[mouse_x][mouse_y+1]==70){//左边无障碍且没有走过
						 turnleft();gostright();
						return 1;
						}
						else if(irC==0&&step[mouse_x+1][mouse_y]==70){//前边无障碍且没有走过
						 gostright();return 1;
						}
						else if(irR==0&&step[mouse_x][mouse_y-1]==70){//右边无障碍且没有走过
							turnright();gostright();
							return 1;
						}
						else 
							return 0;
		case 1:if(irL==0&&step[mouse_x+1][mouse_y]==70){//左边无障碍且没有走过
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x][mouse_y-1]==70){//前边无障碍且没有走过
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x-1][mouse_y]==70){//右边无障碍且没有走过
							turnright();gostright();
							return 1;
						}
					 else
						 return 0;
		case 2:if(irL==0&&step[mouse_x][mouse_y-1]==70){//左边无障碍且没有走过
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x-1][mouse_y]==70){//前边无障碍且没有走过
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x][mouse_y+1]==70){//右边无障碍且没有走过
							turnright();gostright();
							return 1;
						}	
					 else 
						 return 0;
		case 3:if(irL==0&&step[mouse_x-1][mouse_y]==70){//左边无障碍且没有走过
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x][mouse_y+1]==70){//前边无障碍且没有走过
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x+1][mouse_y]==70){//右边无障碍且没有走过
							turnright();gostright();
							return 1;
						}	
					 else 
						 return 0;
	}
}
void 	navigate(){//寻迹函数
	unsigned char temp=0;
	while(1){
		if(go_next())//遍历函数
			;//空语句
		else{
			temp = take_direction()-l_absolute_direction;//用于转弯
			if(temp == 0){
				turnright();
				turnright();
			}
			else if(temp==1)
				turnleft();			
			else if(temp==-1)
				turnright();
			else if(temp==3)
				turnright();
			else if(temp==-3)
				turnleft();
		backstright();
		}
		if(mouse_x==0&&mouse_y==0){
			break;
		}
	}
}
//*****************************************************************算法：冲刺部分******************************************************************
void Contour_table(unsigned char start_x,unsigned char start_y){//生成等高表
    unsigned char c_x,c_y,x,y,i,y_n;
    move_int();step_init();
		maze[0][0]=0x0a;
    queue_in(start_x,start_y);
    step[start_x][start_y] = count_step;
    while (front!=rear){
        c_x = queue[front].x;
        c_y = queue[front].y;
				count_step=step[c_x][c_y]+1;
        for (i = 0; i < 4; i++){
            x = c_x+move[i].x;
            y = c_y+move[i].y;
					  y_n = take_current(c_x,c_y,i);
						if(x>=0&&y>=0&&x<8&&y<8&&y_n&&step[x][y]>count_step){//判断下一个步数的条件
									queue_in(x,y);
									step[x][y]=count_step;
						}
        }
				queue_out();
    }
}
void sprint(unsigned char end_x,unsigned char end_y){//冲刺
	unsigned char end,x,y,i,j,temp,s=0,y_n;
	//用栈存储最短路径
	end=step[end_x][end_y];
	push(end_x,end_y);
	for(front=0;front<8;front++)
		for(rear=0;rear<8;rear++)
			for (i = 0; i < 4; i++) {
				x = end_x+move[i].x;
				y = end_y+move[i].y;
				y_n=take_current(end_x,end_y,i);
				if(x>=0&&y>=0&&x<=end_x&&y<=end_y){
					temp = end - step[x][y];
					if (temp==1&&y_n){
							push(x, y);
							end_x = x;
							end_y = y;
							end=step[end_x][end_y];
							break;
					}
			}
		}
		
		//冲刺阶段此时小车相对方向应该和绝对方向一致
		while(stack_top>1){//冲刺阶段
			i=stack_x[stack_top];
			j=stack_y[stack_top];
			pop();
			end_x=stack_x[stack_top];
			end_y=stack_y[stack_top];
			if(end_y>j)
				s=3;
			else if(end_x<i)
				s=2;
			else if(end_y<j)
				s=1;
			else if(end_x>i)
				s=0;
			temp=s-c_absolute_direction;
			if(temp==1)
					turnright();			
			else if(temp==-1)
					turnleft();
			else if(temp==3)
					turnleft();
			else if(temp==-3)
					turnright();
			else if(temp==2){
					turnleft();
					turnleft();
			}
			else if(temp==-2){
					turnleft();
					turnleft();
			}
			backstright();
	 }
}
void main(){
	Time2_init();
	maze_init();
	step_init();
	delay_ms(100000);
	navigate();
	Contour_table(0,0);
	sprint(7,7);
	while(1);
}
//**************************************************************定时器中断部分-红外检测************************************************************
void Timer2_fix() interrupt 5 { //定时器中断函数
	static bit ir = 0; //标志本次红外是发射还是接收
	unsigned char i,j;
	TF2=0;
	tube1_on(mouse_x);
	tube2_on(mouse_y);
	if(!ir){
		MOUSE_IR_ON(counter_IR-1);
	}
	else {
		if(counter_IR==1){
			if(IR1)
				irC=0;
			else
				irC=1;
		}
		else if(counter_IR==3){
				if(IR3)
					irL=0;
				else
					irL=1;
			}
		else if(counter_IR==4){
			if(IR4)
				irR=0;
			else
				irR=1;
		}
		else if(counter_IR==2){
				if(IR2)
					irLU=0;		
				else{
					irLU=1;
					for(j=0;j<4;j++){//姿态调整部分
						for(i=0;i<8;i++){
							P1=positive[i];
							delay_ms(3);
						}
					}
				}
			}
		else if(counter_IR==5){
			if(IR5)
				irRU=0;
			else{
				irL=1;
				for(j=0;j<4;j++){//姿态调整部分
						for(i=0;i<8;i++){
							P1=counter[i];
							delay_ms(3);
						}
					}
			}
		}
	}
	if(ir)
		counter_IR++;
	if(counter_IR>5)
		counter_IR=1;
	ir=~ir;
}