
// Der Mond ist aufgegangen

unsigned char songNotes[] = { 
	65, 193, 67, 195, 65, 193, 70, 198, 69, 197, 67, 195, 65, 193, 69, 197, 
	69, 197, 69, 197, 74, 202, 72, 200, 70, 198, 69, 197, 69, 197, 69, 197, 
	69, 197, 70, 198, 69, 197, 67, 195, 65, 193, 67, 195, 65, 193, 70, 198, 
	69, 197, 67, 195, 65, 193, 69, 197, 69, 197, 69, 197, 74, 202, 72, 200, 
	70, 198, 69, 197, 69, 197, 69, 197, 69, 197, 70, 198, 69, 197, 67, 195, 
	67, 195, 65, 193, 65, 193, 67, 195, 65, 193, 70, 198, 69, 197, 67, 195, 
	65, 193, 69, 197, 69, 197, 69, 197, 74, 202, 72, 200, 70, 198, 69, 197, 
	69, 197, 69, 197, 69, 197, 70, 198, 69, 197, 67, 195, 65, 193, 67, 195, 
	65, 193, 70, 198, 69, 197, 67, 195, 65, 193, 69, 197, 69, 197, 69, 197, 
	74, 202, 72, 200, 70, 198, 69, 197, 69, 197, 69, 197, 69, 197, 70, 198, 
	69, 197, 67, 195, 67, 195, 65, 193, 65, 193, 67, 195, 65, 193, 70, 198, 
	69, 197, 67, 195, 65, 193, 69, 197, 69, 197, 69, 197, 74, 202, 72, 200, 
	70, 198, 69, 197, 69, 197, 69, 197, 69, 197, 70, 198, 69, 197, 67, 195, 
	65, 193, 67, 195, 65, 193, 70, 198, 69, 197, 67, 195, 65, 193, 69, 197, 
	69, 197, 69, 197, 74, 202, 72, 200, 70, 198, 69, 197, 69, 197, 69, 197, 
	69, 197, 70, 198, 69, 197, 67, 195, 67, 195, 65, 193, 65, 193, 67, 195, 
	65, 193, 70, 198, 69, 197, 67, 195, 65, 193, 69, 197, 69, 197, 69, 197, 
	74, 202, 72, 200, 70, 198, 69, 197, 69, 197, 69, 197, 69, 197, 70, 198, 
	69, 197, 67, 195, 65, 193, 67, 195, 65, 193, 70, 198, 69, 197, 67, 195, 
	65, 193, 69, 197, 69, 197, 69, 197, 74, 202, 72, 200, 70, 198, 69, 197, 
	69, 197, 69, 197, 69, 197, 70, 198, 69, 197, 67, 195, 67, 195, 65, 193
};

unsigned short songTimeDiff[] = { 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 384, 336, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 
	30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 384, 336, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 960, 240, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 384, 
	336, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 240, 0, 450, 30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 
	0, 240, 0, 384, 336, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 
	30, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 450, 30, 240, 
	0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 240, 0, 960
};
unsigned short noteCount = 336;


