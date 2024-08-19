//
// Created by zenglj on 2021/8/23.
//

#ifndef MINIC_STD_H
#define MINIC_STD_H

/* Input & output functions */
int getint();
int getch();
int getarray(int a[]);

void putint(int a);
void putch(int a);
void putarray(int n,int a[]);

void putstr(char * str);

float getfloat();
int getfarray(float a[]);
void putfloat(float a);
void putfarray(int n, float a[]);
void putf(char a[], ...);

#endif //MINIC_STD_H
