/*
 * joysticMouse.hpp
 *
 *  Created on: Mar 20, 2019
 *      Author: ubuntu
 */

#ifndef JOYSTICMOUSE_HPP_
#define JOYSTICMOUSE_HPP_

#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <osa.h>

class CJoystickMouse
{
public:
	CJoystickMouse(const std::string& tag = "X", int screenSize = 1920):m_tag(tag), m_screenSize(screenSize){
		m_pos = m_screenSize>>1;
		m_speed = 0.f;
		m_input = 0.f;
		m_stamp = cv::getTickCount();
		m_last = m_stamp;
		m_bfirst = true;
		m_bZero = false;
	};
	virtual ~CJoystickMouse(){}
	bool update(float x)
	{
		bool ret = m_bfirst;
		float speedBak = m_speed;
		int64 curStamp = cv::getTickCount();
		m_bfirst = false;
		if(fabs(x)<1e-5 || ret){
			m_speed = 0.f;
			m_input = 0.0f;
			m_stamp = curStamp;
			m_bZero = false;
			return (ret||(fabs(m_speed-speedBak)>1e-5));
		}

		double elapsed = (curStamp - m_stamp)*0.000000001;
		float curSpeed = (x - m_input)/elapsed;

		if(fabs(curSpeed)<1e-5 && fabs(m_speed)>1e-5){
			m_speed -= (fabs(m_speed)*0.25f)*(m_speed/fabs(m_speed));
		}else{
			if(fabs(m_speed)>1e-5){
				if(m_speed * curSpeed <= 1e-5){
					m_speed = 0.0f;
					m_bZero = true;
				}else{
					m_speed = curSpeed;
				}
			}else{
				if(!m_bZero)
					m_speed = curSpeed;
			}
		}

		int pos = m_pos;
		elapsed = 0.1f;//(curStamp - m_last)*0.000000001;
		m_pos += m_speed*elapsed;
		m_pos = std::min(std::max(m_pos,0), m_screenSize);

		if(fabs(x-m_input)>1e-5)
			m_stamp = curStamp;
		m_last = curStamp;
		m_input = x;

		ret = (pos != m_pos || ret);

		//if(ret || fabs(speedBak - m_speed)>1e-5)
		//OSA_printf("[%d]%s input (%f) speed %f(%f) pos(%d)", OSA_getCurTimeInMsec(), m_tag.c_str(),
		//			x, m_speed, curSpeed, m_pos);

		return ret;
	}

	std::string m_tag;
	float m_speed;
	int m_screenSize;
	int m_pos;

protected:
	float m_input;
	int64 m_stamp;
	int64 m_last;

private:
	bool m_bfirst;
	bool m_bZero;

};

class CJoystickMouseHK
{
public:
	CJoystickMouseHK(const cv::Size &screenSize = cv::Size(1920, 1080)):m_screenSize(screenSize){
		m_mouse[0] = new CJoystickMouse("X", m_screenSize.width);
		m_mouse[1] = new CJoystickMouse("Y", m_screenSize.height);
	}
	virtual ~CJoystickMouseHK(){
		delete m_mouse[0];
		delete m_mouse[1];
	}
	bool update(unsigned char x, unsigned char y, cv::Point &to){
		bool ret = false;
		float fx = 0.0f;
		float fy = 0.0f;

		if(x & 0x80)
			fx = -1.0*((~x+1)&0x0f)*(m_screenSize.width/16.0);
		else
			fx = 1.0*(x&0x0f)*(m_screenSize.width/16.0);

		ret |= m_mouse[0]->update(fx);

		if(y & 0x80)
			fy = -1.0*((~y+1)&0x0f)*(m_screenSize.height/16.0);
		else
			fy = 1.0*(y&0x0f)*(m_screenSize.height/16.0);

		ret |= m_mouse[1]->update(fy);

		to.x = m_mouse[0]->m_pos;
		to.y = m_mouse[1]->m_pos;

		if(0){
			OSA_printf("[%d]%s input(0x%02x,0x%02x)(%f,%f) speed(%f,%f) pos(%d,%d)", OSA_getCurTimeInMsec(), __func__,
					x,y, fx, fy, m_mouse[0]->m_speed, m_mouse[1]->m_speed, to.x, to.y);
		}

		return ret;
	}
protected:
	cv::Size m_screenSize;
	CJoystickMouse *m_mouse[2];
};





#endif /* JOYSTICMOUSE_HPP_ */
