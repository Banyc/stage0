/* Copyright (C) 2016 Jeremiah Orians
 * This file is part of stage0.
 *
 * stage0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * stage0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with stage0.  If not, see <http://www.gnu.org/licenses/>.
 */

#define VM_H__VAR_DEF
#include "vm.h"
#include <getopt.h>

/* Load program tape into Memory */
size_t load_program(struct lilith* vm, char* rom_name)
{
	FILE* program;
	program = fopen(rom_name, "r");
	if(NULL == program)
	{
		fprintf(stderr, "The rom file: %s could not be found\nif you are running the make; you forgot the git submodules\nPlease review the section: \"Need to know information\" in the README\n", rom_name);
		exit(EXIT_FAILURE);
	}

	/* Figure out how much we need to load */
	fseek(program, 0, SEEK_END);
	size_t end = ftell(program);
	rewind(program);

	/* Deal with the special case of the ROM is bigger than available memory */
	if(end > vm->amount_of_Ram)
	{
		fprintf(stderr, "Program %s is %d bytes large but only %d bytes of memory have been allocated!\n", rom_name, (int)end, (int)vm->amount_of_Ram);
		exit(EXIT_FAILURE);
	}

	/* Load the entire tape into memory */
	fread(vm->memory, 1, end, program);

	fclose(program);
	return end;
}

void execute_vm(struct lilith* vm)
{
	struct Instruction* current;
	current = calloc(1, sizeof(struct Instruction));

	while(!vm->halted)
	{
		read_instruction(vm, current);
		eval_instruction(vm, current);
	}

	free(current);
	return;
}

/* Standard C main program */
int main(int argc, char **argv)
{
	POSIX_MODE = false;
	FUZZING = false;
	char* c;
	int length;
	int Memory_Size = (16 * 1024);

	tape_01_name = "tape_01";
	tape_02_name = "tape_02";
	char* rom_name = NULL;
	char class;

	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-r") || match(argv[i], "--rom"))
		{
			rom_name = argv[i + 1];
			i = i + 2;
		}
		else if(match(argv[i], "-h") || match(argv[i], "--help"))
		{
			fprintf(stdout, "Usage: %s --rom $rom [--tape_01 $foo] [--tape_02 $bar]\n", argv[0]);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "-1") || match(argv[i], "--tape_01"))
		{
			tape_01_name = argv[i + 1];
			i = i + 2;
		}
		else if(match(argv[i], "-2") || match(argv[i], "--tape_02"))
		{
			tape_02_name = argv[i + 1];
			i = i + 2;
		}
		else if(match(argv[i], "-m") || match(argv[i], "--memory"))
		{
			length = strlen(argv[i + 1]) - 1;
			c = argv[i+1];
			class = c[length];
			c[length] = 0;
			Memory_Size = atoi(c);
			if('K' == class)
			{
				Memory_Size = Memory_Size * 1024;
			}
			else if('M' == class)
			{
				Memory_Size = Memory_Size * 1024 * 1024;
			}
			else if('G' == class)
			{
				Memory_Size = Memory_Size * 1024 * 1024 * 1024;
			}
			i = i + 2;
		}
		else if(match(argv[i], "-P") || match(argv[i], "--POSIX-MODE"))
		{
				POSIX_MODE = true;
				i = i + 1;
		}
		else if(match(argv[i], "-F") || match(argv[i], "--fuzzing"))
		{
			FUZZING = true;
			i = i + 1;
		}
		else
		{
			fprintf(stderr, "unknown option %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	if(NULL == rom_name)
	{
		fprintf(stderr, "Usage: %s --rom $rom [--tape_01 $foo] [--tape_02 $bar]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Perform all the essential stages in order */
	struct lilith* vm;
	size_t image;
	vm = create_vm(Memory_Size);
	image = load_program(vm, rom_name);
	if(POSIX_MODE) vm->reg[15] = image;
	execute_vm(vm);
	destroy_vm(vm);

	return EXIT_SUCCESS;
}
