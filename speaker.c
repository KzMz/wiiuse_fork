#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "definitions.h"
#include "wiiuse_internal.h"

void wiiuse_set_speaker(struct wiimote_t *wm,int status)
{
	byte conf[7] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	byte buf = 0x00;

	if(!wm) return;

	if(!WIIMOTE_IS_SET(wm,WIIMOTE_STATE_HANDSHAKE_COMPLETE)) {
		WIIUSE_DEBUG("Tried to enable speaker, will wait until handshake finishes.\n");
		if(status)
			WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
		else
			WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
		return;
	}

	if(status) {
		if(WIIMOTE_IS_SET(wm,WIIMOTE_STATE_SPEAKER)) {
			return;
		}
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
	} else {
		if(!WIIMOTE_IS_SET(wm,WIIMOTE_STATE_SPEAKER)) {
			return;
		}
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
	}


	buf = 0x04;
	wiiuse_send(wm,WM_CMD_SPEAKER_ENABLE,&buf,1);

	if (!status) {
		WIIUSE_DEBUG("Disabled speaker for wiimote id %i.", wm->unid);

		buf = 0x01;
		wiiuse_write_data(wm,WM_REG_SPEAKER_REG1,&buf,1);

		buf = 0x00;
		wiiuse_write_data(wm,WM_REG_SPEAKER_REG3,&buf,1);

		buf = 0x00;
		wiiuse_send(wm,WM_CMD_SPEAKER_ENABLE,&buf,1);

		wiiuse_set_report_type(wm);
		return;
	}

	buf = 0x04;
	wiiuse_send(wm,WM_CMD_SPEAKER_MUTE,&buf,1);

	buf = 0x01;
	wiiuse_write_data(wm,WM_REG_SPEAKER_REG3,&buf,1);

	buf = 0x08;
	wiiuse_write_data(wm,WM_REG_SPEAKER_REG1,&buf,1);

	//conf[1] = 0xd0;
	conf[3] = 0x07;
	conf[4] = 0x40;
	wiiuse_write_data(wm,WM_REG_SPEAKER_BLOCK,conf,7);

	buf = 0x01;
	wiiuse_write_data(wm,WM_REG_SPEAKER_REG2,&buf,1);

	buf = 0x00;
	wiiuse_send(wm,WM_CMD_SPEAKER_MUTE,&buf,1);

	wiiuse_set_report_type(wm);
	return;
}

void wiiuse_play_sound(struct wiimote_t* wm, byte* data, int size) {
	int written = 0, to_be_written;
	byte * data_ptr = data;
	if(data != NULL && size > 0) {
		while(written < size) {
			to_be_written = size - written;
			if(to_be_written > 21) to_be_written = 21;
			wiiuse_send(wm, WM_CMD_SPEAKER_DATA, data_ptr, to_be_written);
			data_ptr += to_be_written;
			written += to_be_written;
		}
	}
	return;
}
