#include "main.h"

bit irC=0,irL=0,irR=0,irLU=0,irRU=0;//������⴫�������״̬ȫ��λ������Ϊ0���ϰ�
unsigned char counter_IR=1;
unsigned char code table[] = {0xc0,0xf9,0xa4,0xb0,0x99, 0x92,0x82,0xf8,0x80,0x90};//�����������ʾ����
unsigned char code forword[] = {0x11,0x93,0x82,0xc6,0x44,0x6c,0x28,0x39};//ֱ������
unsigned char code positive[] = {0x11,0x33,0x22,0x66,0x44,0xcc,0x88,0x99};//˳ʱ��ת-��ת
unsigned char code counter[] = {0x11,0x99,0x88,0xcc,0x44,0x66,0x22,0x33};//��ʱ��ת-��ת
unsigned char l_absolute_direction=3,c_absolute_direction=3,relative_direction=0;//���Է������Է���
//*******************************************************************׼�����֣�ջ������***********************************************************
//maze
unsigned char maze[8][8];//��ͼ����
unsigned char step[8][8];//��������
unsigned char mouse_x=0,mouse_y=0;//�Թ�������
unsigned char count_step=0;       //����
void MOUSE_IR_ON(GROUP_NO) {//�����⺯��
	do{ 
		A0=(GROUP_NO)&0x01;
		A1=(GROUP_NO)&0x02; 
		A2=(GROUP_NO)&0x04;
		}while(0);
}
//stack
unsigned char  stack_x[40];   //����ջ
unsigned char  stack_y[40];   //����ջ
unsigned char  stack_top = 0; //ջ��
void push(unsigned char x,unsigned char y){//���
		stack_top++;
		stack_x[stack_top] = x;  
		stack_y[stack_top] = y;  
}
void pop(){//����
	stack_top--;
}
//queue
typedef struct{//����ṹ��
    unsigned char x;
    unsigned char y;
}coordinate;
coordinate queue[10];  //��������
coordinate move[4];    //��������
unsigned char front=0,rear=0;      //��ͷ�Ͷ�β
void queue_in(unsigned char in_x,unsigned char in_y){//��ջ
    queue[rear].x = in_x;
    queue[rear].y = in_y;
    rear=(rear+1)%10;
}
void queue_out(){//��ջ
    front=(front+1)%10;
}
void move_int(){//��ʼ�������ж�һ�����ӵ��ĸ�����
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
void delay_ms(unsigned int z){//��ʱ����
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
void  Time2_init(){//��ʱ����ʼ������
	EA=1;
	ET2=1;
	TH2=RCAP2H=(65536-7000)/256; 
	TL2=RCAP2L=(65536-7000)%256;
	TR2=1;
}
//*******************************************************************������������*****************************************************************
void beep_on(){//������
	Beep = 0;
	delay_ms(10000);
	Beep = 1;
	delay_ms(10000);
}
void tube1_on(unsigned char x){//�����1
	tube1 = 0;
	tube2 = 1;
	P0 = table[x];
}
void tube2_on(unsigned char x){//�����2
	tube1 = 1;
	tube2 = 0;
	P0 = table[x];
}

//*******************************************************************�����������*****************************************************************
void gostright(){//����ʱ��ֱ�ߺ�����������¼��Ϣ
	unsigned char i,j;
	delay_ms(1500);//��һ����ʱ
	for(j=0;j<104;j++){	//j����������
			for(i=0;i<8;i++){
				P1=(forword[i]);
				delay_ms(80);
			}
		}
	update();
	maze[mouse_x][mouse_y] &= note_current();//��¼�Թ�ǽ����Ϣ
	maze[mouse_x][mouse_y] &= note_current();//��¼�Թ�ǽ����Ϣ
	note_direction();//��¼����·��
	delay_ms(1500);//��һ����ʱ
}
void backstright(){//����ʱֱ�ߺ�����ֱֻ�߲���¼��Ϣ
	unsigned char i,j;
	for(j=0;j<104;j++){	//j����������
			for(i=0;i<8;i++){
				P1=(forword[i]);
				delay_ms(80);
			}
		}
	update();
	delay_ms(1500);//��һ����ʱ
}
void turnleft(){ //��ת 
	unsigned char i,j;	//j����������
	delay_ms(1000);//��һ����ʱ
	TR2=0;
	for(j=0;j<50;j++){
		for(i=0;i<8;i++){
			P1=counter[i];
			delay_ms(20);
		}
	}
	c_absolute_direction = (l_absolute_direction+3)%4;//��Է��򵽾��Է����ת��
	l_absolute_direction = c_absolute_direction;
	TR2=1;
	delay_ms(1000);//��һ����ʱ
}
void turnright(){//��ת 
	unsigned char i,j;	//j����������
	delay_ms(1000);//��һ����ʱ
	TR2=0;
	for(j=0;j<50;j++){
		for(i=0;i<8;i++){
			P1=positive[i];
			delay_ms(20);
		}
	}
	c_absolute_direction = (l_absolute_direction+1)%4;//��Է��򵽾��Է����ת��
	l_absolute_direction = c_absolute_direction;
	TR2=1;
	delay_ms(1000);//��һ����ʱ
}
//*******************************************************************���ܺ�������*****************************************************************
void maze_init(){//��ͼ�����ʼ��
	unsigned char i,j;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++)
			maze[i][j] = 0xff;
	}
}
void step_init(){//����������ʼ��
	unsigned char i,j;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++)
			step[i][j] = 70;
	}
}
unsigned char note_current(){//��ǰ�Թ���ǽ����Ϣ->����λ ��0��ʾû�е��壬1��ʾ�е���(����λ��ʱΪ1)
	unsigned char current;
	switch(c_absolute_direction){
		case 0:	if(irC==1&&irR==1&&irL==1)//��·
							current = 0x0d;
						else if(irC==0&&irR==1&&irL==1)//ǰ�����ϰ�
							current = 0x05;
						else if(irC==0&&irR==1&&irL==0)//ǰ�ߺ�������ϰ�
							current = 0x04;
						else if(irC==0&&irR==0&&irL==1)//ǰ�ߺ��ұ����ϰ�
							current = 0x01;
						else if(irC==1&&irR==0&&irL==1)//�ұ����ϰ�
							current = 0x09;
						else if(irC==1&&irR==1&&irL==0)//������ϰ�
							current = 0x0c;
						else if(irC==1&&irR==0&&irL==0)//�ұߺ�������ϰ�
							current = 0x08;
						else if(irC==0&&irR==0&&irL==0)//�������ϰ�
							current = 0x00;break;
		case 1:	if(irC==1&&irR==1&&irL==1)//��·
							current = 0x0e;
						else if(irC==0&&irR==1&&irL==1)//ǰ�����ϰ�
							current = 0x0a;
						else if(irC==0&&irR==1&&irL==0)//ǰ�ߺ�������ϰ�
							current = 0x02;
						else if(irC==0&&irR==0&&irL==1)//ǰ�ߺ��ұ����ϰ�
							current = 0x08;
						else if(irC==1&&irR==0&&irL==1)//�ұ����ϰ�
							current = 0x0c;
						else if(irC==1&&irR==1&&irL==0)//������ϰ�
							current = 0x06;
						else if(irC==1&&irR==0&&irL==0)//�ұߺ�������ϰ�
							current = 0x04;
						else if(irC==0&&irR==0&&irL==0)//�������ϰ�
							current = 0x00;break;
		case 2:	if(irC==1&&irR==1&&irL==1)//��·
							current = 0x07;
						else if(irC==0&&irR==1&&irL==1)//ǰ�����ϰ�
							current = 0x05;
						else if(irC==0&&irR==1&&irL==0)//ǰ�ߺ�������ϰ�
							current = 0x01;
						else if(irC==0&&irR==0&&irL==1)//ǰ�ߺ��ұ����ϰ�
							current = 0x04;
						else if(irC==1&&irR==0&&irL==1)//�ұ����ϰ�
							current = 0x04;
						else if(irC==1&&irR==1&&irL==0)//������ϰ�
							current = 0x03;
						else if(irC==1&&irR==0&&irL==0)//�ұߺ�������ϰ�
							current = 0x02;
						else if(irC==0&&irR==0&&irL==0)//�������ϰ�
							current = 0x00;break;
		case 3:	if(irC==1&&irR==1&&irL==1)//��·
							current = 0x0b;
						else if(irC==0&&irR==1&&irL==1)//ǰ�����ϰ�
							current = 0x0a;
						else if(irC==0&&irR==1&&irL==0)//ǰ�ߺ�������ϰ�
							current = 0x08;
						else if(irC==0&&irR==0&&irL==1)//ǰ�ߺ��ұ����ϰ�
							current = 0x02;
						else if(irC==1&&irR==0&&irL==1)//�ұ����ϰ�
							current = 0x03;
						else if(irC==1&&irR==1&&irL==0)//������ϰ�
							current = 0x09;
						else if(irC==1&&irR==0&&irL==0)//�ұߺ�������ϰ�
							current = 0x01;
						else if(irC==0&&irR==0&&irL==0)//�������ϰ�
							current = 0x00;break;
					}
	return current;
}
void note_direction(){//��¼��һ���Թ�������Թ���ľ��Է��� ->����λ��(0��ʾ�����������,1��ʾ�������������)
		step[mouse_x][mouse_y]=c_absolute_direction;
}
void update(){//�����ˢ��
	if(l_absolute_direction == 0)//�Թ���0
			mouse_x +=1;
	if(l_absolute_direction == 1)//�Թ���1
			mouse_y -=1;
	if(l_absolute_direction == 2)//�Թ���2
			mouse_x -=1;
	if(l_absolute_direction == 3)//�Թ���3
			mouse_y +=1;
}
unsigned char take_direction(){//ȡ���뵱ǰ����ʱ�ķ��򡪡�>�������
	unsigned char direction;//�����λ����Ϣ
	direction=step[mouse_x][mouse_y];
	return direction;
}
unsigned char take_current(unsigned char x,unsigned char y,unsigned char i){//ȡ��ǰ�Թ���Ӧ�����Ƿ�����ߣ�û��ǽ����1����ǽ����0���ڻ���
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
//****************************************************************�㷨����������*******************************************************************
unsigned char go_next(){//����һ����������1��0�Ƿ����
	switch(c_absolute_direction){
		case 0:if(irL==0&&step[mouse_x][mouse_y+1]==70){//������ϰ���û���߹�
						 turnleft();gostright();
						return 1;
						}
						else if(irC==0&&step[mouse_x+1][mouse_y]==70){//ǰ�����ϰ���û���߹�
						 gostright();return 1;
						}
						else if(irR==0&&step[mouse_x][mouse_y-1]==70){//�ұ����ϰ���û���߹�
							turnright();gostright();
							return 1;
						}
						else 
							return 0;
		case 1:if(irL==0&&step[mouse_x+1][mouse_y]==70){//������ϰ���û���߹�
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x][mouse_y-1]==70){//ǰ�����ϰ���û���߹�
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x-1][mouse_y]==70){//�ұ����ϰ���û���߹�
							turnright();gostright();
							return 1;
						}
					 else
						 return 0;
		case 2:if(irL==0&&step[mouse_x][mouse_y-1]==70){//������ϰ���û���߹�
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x-1][mouse_y]==70){//ǰ�����ϰ���û���߹�
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x][mouse_y+1]==70){//�ұ����ϰ���û���߹�
							turnright();gostright();
							return 1;
						}	
					 else 
						 return 0;
		case 3:if(irL==0&&step[mouse_x-1][mouse_y]==70){//������ϰ���û���߹�
						 turnleft();gostright();
						 return 1;
						}
					 else if(irC==0&&step[mouse_x][mouse_y+1]==70){//ǰ�����ϰ���û���߹�
						 gostright();return 1;
					 }
					 else if(irR==0&&step[mouse_x+1][mouse_y]==70){//�ұ����ϰ���û���߹�
							turnright();gostright();
							return 1;
						}	
					 else 
						 return 0;
	}
}
void 	navigate(){//Ѱ������
	unsigned char temp=0;
	while(1){
		if(go_next())//��������
			;//�����
		else{
			temp = take_direction()-l_absolute_direction;//����ת��
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
//*****************************************************************�㷨����̲���******************************************************************
void Contour_table(unsigned char start_x,unsigned char start_y){//���ɵȸ߱�
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
						if(x>=0&&y>=0&&x<8&&y<8&&y_n&&step[x][y]>count_step){//�ж���һ������������
									queue_in(x,y);
									step[x][y]=count_step;
						}
        }
				queue_out();
    }
}
void sprint(unsigned char end_x,unsigned char end_y){//���
	unsigned char end,x,y,i,j,temp,s=0,y_n;
	//��ջ�洢���·��
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
		
		//��̽׶δ�ʱС����Է���Ӧ�ú;��Է���һ��
		while(stack_top>1){//��̽׶�
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
//**************************************************************��ʱ���жϲ���-������************************************************************
void Timer2_fix() interrupt 5 { //��ʱ���жϺ���
	static bit ir = 0; //��־���κ����Ƿ��仹�ǽ���
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
					for(j=0;j<4;j++){//��̬��������
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
				for(j=0;j<4;j++){//��̬��������
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