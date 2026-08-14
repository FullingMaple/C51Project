/*
 * misc.h
 *
 *  Created on: Nov 24, 2024
 *      Author: tonny
 */

#ifndef MISC_H_
#define MISC_H_

#include "config.h"

#define BTN_LONG_PRESS_MS 1000
#define BTN_DEBOUNCE_MS 50	//����ʱ��ms

typedef struct{
	uint8_t isPressing;				//��ť״̬
	uint8_t isDebouncedPressing;	//�������״̬
	uint8_t isLongPressing;			//�Ƿ����ڳ���
	uint8_t pressEvent;				//δ�����İ����¼�
	uint8_t longPressEvent;			//δ�����ĳ����¼�
	uint32_t pressStartTick;		//����˲���ʱ��
	//uint32_t pressEndTick;		//�ɿ�˲���ʱ��
} BTN_stat_t;

extern BTN_stat_t BTN_stat;

void BTN_init(void);

void BtnTask(void);

#endif /* MISC_H_ */
