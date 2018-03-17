#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <Windows.h>
#else // !_WIN32
#endif // _WIN32

#include "picdec.h"

#pragma pack(push)
#pragma pack(1)
typedef struct Metadata_t {
	char id[4]; // "admv"
	uint16_t meta_size; // = (sizeof(Metadata) + (wcslen(name) + 1) * sizeof(wchar_t) + 0x1FF) & ~0x1FF
	uint16_t meta_ver; // = 0
	uint8_t width; // <= 128
	uint8_t height; // <= 64
	float fps;
	uint32_t frame; //frame count
	uint8_t field; //field count
	wchar_t name[0];
} Metadata;
#pragma pack(pop)

static void printhelp(void) {
	fputs(
		"amvpgen - ArduMan Video Generator\n"
		"usage: amvpgen [args] [outfile]\n"
		"If outfile is not specified, stdout is used.\n"
		"\n"
		"-h --help        Print this help\n"
		"-i --input str   Set input file (raw data)\n"
		"                 - for stdin\n"
		"-x --width int   Set width of raw input\n"
		"-y --height int  Set height of raw input\n"
		"-f --field int   Set field count in one frame\n"
		"-m --meta        Add metadata to output file\n"
		"Following options are available only when -m is used:\n"
		"-t --title str   Set video title to str\n"
		"-r --rate float  Set video rate\n"
		"\n"
		"Any bugs or improvemens, please mail to <AlexGuo1998@163.com>\n"
		, stderr);
}

#ifdef _WIN32
//Get attached console count. if count==1, we are running standalone, show pause.
void exitpause(void) {
	DWORD proc;
	DWORD ret = GetConsoleProcessList(&proc, 1);
	if (ret == 1) {
		system("pause");
	}
}
#else // !_WIN32
#define exitpause() ((void)0)
#endif // _WIN32

int wmain(int argc, wchar_t **argv) {
	if (argc == 1) {
		printhelp();
		exitpause();
		return 1;
	}

	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	wchar_t *finname = NULL, *foutname = NULL;

	int width = 128, height = 64;
	size_t field = 4;//def: 5 levels of gray
	bool usemeta = false;
	wchar_t *title = NULL;
	float rate = 25;

	//parse arg
	for (int i = 1; i < argc; i++) {
		if (wcscmp(argv[i], L"-h") == 0 || wcscmp(argv[i], L"--help") == 0) {
			printhelp();
			exitpause();
			return 0;
		} else if (wcscmp(argv[i], L"-m") == 0 || wcscmp(argv[i], L"--meta") == 0) {
			usemeta = true;
		}
		if (i == argc - 1) {
			foutname = argv[i];
			break;
		}
		if (wcscmp(argv[i], L"-i") == 0 || wcscmp(argv[i], L"--input") == 0) {
			i++;
			finname = argv[i];
		} else if (wcscmp(argv[i], L"-x") == 0 || wcscmp(argv[i], L"--width") == 0) {
			i++;
			int width_new;
			if (swscanf(argv[i], L"%d", &width_new) == 1) {
				width = width_new;
			} else {
				fwprintf(stderr, L"Can't read width value from \"%ws\"!\n", argv[i]);
			}
		} else if (wcscmp(argv[i], L"-y") == 0 || wcscmp(argv[i], L"--height") == 0) {
			i++;
			int height_new;
			if (swscanf(argv[i], L"%d", &height_new) == 1) {
				height = height_new;
			} else {
				fwprintf(stderr, L"Can't read height value from \"%ws\"!\n", argv[i]);
			}
		} else if (wcscmp(argv[i], L"-f") == 0 || wcscmp(argv[i], L"--field") == 0) {
			i++;
			int field_new;
			if (swscanf(argv[i], L"%d", &field_new) == 1) {
				field = field_new;
			} else {
				fwprintf(stderr, L"Can't read frame value from \"%ws\"!\n", argv[i]);
			}
		} else if (wcscmp(argv[i], L"-t") == 0 || wcscmp(argv[i], L"--title") == 0) {
			i++;
			title = argv[i];
		} else if (wcscmp(argv[i], L"-r") == 0 || wcscmp(argv[i], L"--rate") == 0) {
			i++;
			float rate_new;
			if (swscanf(argv[i], L"%f", &rate_new) == 1) {
				rate = rate_new;
			} else {
				fwprintf(stderr, L"Can't read rate value from \"%ws\"!\n", argv[i]);
			}
		}
	}

	FILE *fin, *fout;
	if (finname == NULL) {
		fin = stdin;
	} else {
		fin = _wfopen(finname, L"rb");
		if (fin == NULL) {
			fwprintf(stderr, L"Can't open \"%ws\" for read!\n", finname);
			exitpause();
			return 1;
		}
	}
	if (foutname == NULL) {
		fout = stdout;
	} else {
		fout = _wfopen(foutname, L"wb");
		if (fout == NULL) {
			fwprintf(stderr, L"Can't open \"%ws\" for write!\n", foutname);
			exitpause();
			return 1;
		}
	}

	Metadata meta;
	if (usemeta) {
		memcpy(meta.id, "admv", 4);
		meta.meta_size = (sizeof(Metadata) + (wcslen(title) + 1) * sizeof(wchar_t) + 0x1FF) & ~0x1FF;
		meta.meta_ver = 0;
		meta.width = width;
		meta.height = height;
		meta.fps = rate;
		meta.frame = 0;
		meta.field = (uint8_t)field;

		fwrite(&meta, sizeof(meta), 1, fout);
		fwrite(&title, sizeof(wchar_t), wcslen(title), fout);
		//paddle zeros
		size_t size_remain = meta.meta_size - sizeof(meta) - wcslen(title);
		uint8_t zero = 0;
		for (size_t i = 0; i < size_remain; i++) {
			fwrite(&zero, 1, 1, fout);
		}
	}
	
	size_t insize = width * height * 3;
	size_t outsize = width * ((height + 7) >> 3);
	uint8_t *buffer = _alloca(insize);
	uint8_t *buffer_out = _alloca(outsize);
	uint32_t framecount = 0;
	while (!feof(fin)) {
		if (fread(buffer, insize, 1, fin) != 1) break;
		framecount++;
		for (size_t i = 0; i < field; i++) {
			decode(buffer, width, height, i, field, buffer_out);
			if (fwrite(buffer_out, outsize, 1, fout) != 1) {
				fwprintf(stderr, L"Can't write outfile!\n");
				exitpause();
				return 1;
			}
		}
	}
	fclose(fin);

	if (usemeta && foutname != NULL) {//not stdout, rewind
		meta.frame = framecount;
		rewind(fout);
		fwrite(&meta, sizeof(meta), 1, fout);
	}

	fclose(fout);
	return 0;
}