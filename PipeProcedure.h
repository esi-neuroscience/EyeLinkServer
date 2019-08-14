UINT PipeProcedure( LPVOID pParam );
void WritePipe(void* buffer, unsigned char bytesToWrite);

#define BUFFERSIZE 256

union COMMANDBUFFER {
	unsigned char command[BUFFERSIZE];
	struct {
		WORD key;
		unsigned char body[BUFFERSIZE-2];
	};
};
