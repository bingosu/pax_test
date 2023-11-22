
#include <posapi.h>
#include <posapi_all.h>

const APPINFO AppInfo={
	"POS-Simple example",
	"APP-TEST",
	"1.0",
	"pcteam",
	"demo program",
	"",
	0,
	0,
	0,
	""
};

int event_main(ST_EVENT_MSG *msg)
{
	SystemInit();
	return 0;
}


int main(void)
{
	unsigned char time[7], time2[20], buff[20];
	
	SystemInit();
	
	while (1) {
		//TODO: Start your application
		ScrGotoxy(0, 0);
		ScrAttrSet(1);
		Lcdprintf("  Hello World!  ");
		ScrAttrSet(0);
		ScrGotoxy(0,3);
		Lcdprintf(" Swiped card<<<");
		while (1) {
			GetTime(time);
			if (!memcmp(time, time2, 6)) continue;
			sprintf(buff, "%c%c/%c%c   %c%c:%c%c:%c%c",
				(time[1] >> 4) + 0x30, (time[1] & 0x0f) + 0x30,
				(time[2] >> 4) + 0x30, (time[2] & 0x0f) + 0x30,
				(time[3] >> 4) + 0x30, (time[3] & 0x0f) + 0x30,
				(time[4] >> 4) + 0x30, (time[4] & 0x0f) + 0x30,
				(time[5] >> 4) + 0x30, (time[5] & 0x0f) + 0x30);
			ScrGotoxy(0, 6);
			Lcdprintf(buff);
			memcpy(time2, time, 6);
		}
	}
	return 0;
}
