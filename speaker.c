#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "definitions.h"
#include "wiiuse_internal.h"

void wiiuse_mute_speaker(struct wiimote_t *wm, int status) {
	if(!wm) return;
	
	if(!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_SPEAKER))
		return;
	
	if(WIIMOTE_IS_SET(wm, WIIMOTE_STATE_SPEAKER_MUTE))
		return;
		
	WIIUSE_DEBUG("Muting speaker...");
	
	byte buf = 0x00;
	
	if(status)
		buf = 0x04;
	
	wiiuse_send(wm, WM_CMD_SPEAKER_MUTE, &buf, 1);

	return;
  
}

void wiiuse_set_speaker(struct wiimote_t *wm,int status) {
	if(!wm) return;

	byte buf;
	if(status) {
		if(WIIMOTE_IS_SET(wm,WIIMOTE_STATE_SPEAKER)) {
			return;
		}
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
		buf = 0x04;
	} else {
		if(!WIIMOTE_IS_SET(wm,WIIMOTE_STATE_SPEAKER)) {
			return;
		}
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER_MUTE);
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
		buf = 0x00;
	}

	wiiuse_send(wm,WM_CMD_SPEAKER_ENABLE,&buf,1);

	if (!status) 
		WIIUSE_DEBUG("Disabled speaker for wiimote id %i.", wm->unid);
	else
		WIIUSE_DEBUG("Enabled speaker for wiimote id %i.", wm->unid);
		        
	return;
}

void wiiuse_play_sound(struct wiimote_t* wm, byte* data, int size, byte freq) {
	if(!wm) return;
	if(!data) return;
	
	byte* sound_report[21];
	int written = 0;
	int frame = 0;
	//TODO: let's change volume!
	byte vol = 0x40;
	struct timeval start, end; // *nix only
	float frame_ms = 1000/(freq_hz/ 40.0);
	int to_be_written, left, entries;
	byte* data_ptr = data;
	
	if(WIIMOTE_IS_SET(wm, WIIMOTE_STATE_SPEAKER_PLAYING))
		return; //TODO: sound queue
		
	wiiuse_set_speaker(wm, 1);
	wiiuse_mute_speaker(wm, 1);
	
	byte conf[7] = { 0x00, 0x00, 0x00, freq, vol, 0x00, 0x00 };
	
	byte buf = 0x01;
	wiiuse_write_data(wm, WM_REG_SPEAKER_REG3, buf, 1);
	
	buf = 0x08;
	wiiuse_write_data(wm, WM_REG_SPEAKER_REG1, buf, 1);
	
	wiiuse_write_data(wm, WM_REG_SPEAKER_REG1, conf, 7);
	
	buf = 0x01;
	wiiuse_write_data(wm, WM_REG_SPEAKER_REG2, buf, 1);
	
	wiiuse_mute_speaker(wm, 0);
	
	short i = 0;
	for(; i < 21; i++)
		sound_report[i] = 0x00;
	
	gettimeofday(&start, 0);
	
	WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER_PLAYING);
	
	//TODO: let's play in threads!
	while(written < size) {
		left = size - written;
		if(left < 40)
			to_be_written = left;
		else
			to_be_written = 40;
			
		entries = (to_be_written + 1) >> 1;
						
		sound_report[0] = (byte)(entries<<3);
		
		i = 0;
		for(; i < entries; i++) .
			sound_report[1 + i] = data_ptr[i];
		wiiuse_send(wm, WM_CMD_SPEAKER_DATA, sound_report, 21);
		data_ptr += to_be_written / 2;
		
		frame++;
		
		gettimeofday(&end, 0);
		while((end.tv_sec - start.tv_sec) < (int)(frame * frame_ms))
			sleep(1);
	}
	
	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_SPEAKER_PLAYING);
	return;
}
