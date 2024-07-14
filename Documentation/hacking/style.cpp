//========= Copyright N11 Software, All rights reserved. ============//
//
// File: Style.cpp
// Purpose: The Inferno kernel source code style guide.
// Maintainer: aristonl
//
//===================================================================//

// The file header appears first with a blank line after.

/*
 * This style guide covers the source code styling used for the Inferno
 * kernel.  This style guide only covers any kernel-related code, as
 * well as drivers and basic libraries. Anything used in userspace
 * (e.g, programs in /bin) does not need to follow the style guide.
 */

/*
 * VERY important single-line comments look like this.
 */

// Most single-line comments look like this.

/*
 * Multi-line comments look like this.  Make them real sentences.  Fill
 * them so they look like real paragraphs. Still try to stay under 72
 * characters.
 */

/*
 * A header file should protect itself from multiple inclusions.
 * Use #pragma once for this.
 */
  
#pragma once

/*
 * Kernel headers come first. These should be organised alphabetically.
 */

#include <BOB.h>
#include <COM.h>
#include <Config.h>
#include <GDT.h>

/*
 * If driver headers are used, include them next, after a blank line.
 */

#include <drivers/rtc/rtc.h>
#include <drivers/pci/pci.h>

/*
 * If you're using any /usr include files, include them here after a
 * blank line. These should also be sorted lexicographicallu.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Any functions that aren't used anywhere else are declared at the top
 * of the source file.
 */

static char *Function(int a, int b, float c, int d);

/*
 * Tabs should always be used in place of spaces when indenting code.
 * Indent size is 4 spaces. This is a good indicator when have
 * over-engineered your code. If you need 3-6 levels of indentation,
 * you might want to consider fixing parts of your code; over 6 levels
 * and your best bet is to restart the whole thing and write it again.
 */

int Foo(void) {
	int Bar = 0;
	int Baz = 0;
	int Qux = 0;
}

/*
 * Brackets should always be on the same line as the parent statement.
 */

if (Foo == true) {

}

int main(int x) {
	
}

/*
 * Children properties in namespaces require indentation.
 */

namespace APIC {
	void Write();
	unsigned int Read();
	void Enable();
}

/*
 * Case statements should be indented.
 */

switch (pd.size) {
	case 0:
		signedNum = va_arg(args, int);
		break;
	case 1:
		signedNum = va_arg(args, long);
		break;
}

/*
 * Hash statements are not indented.
 */

void Inferno(BOB* bob) {
#if enableGDT == true
	GDT::Table GDT = {
		{ 0, 0, 0, 0x00, 0x00, 0 },
		{ 0, 0, 0, 0x9a, 0xa0, 0 },
		{ 0, 0, 0, 0x92, 0xa0, 0 },
		{ 0, 0, 0, 0xfa, 0xa0, 0 },
		{ 0, 0, 0, 0xf2, 0xa0, 0 },
	}
	GDT::Descriptor descriptor;
	descriptor.size = sizeof(GDT) - 1;
	descriptor.offset = (unsigned long long)&GDT;
	LoadGDT(&descriptor);
#endif
}

/*
 * Condense any short statements to one line.
 */

if (Foo == 1) return 1;

/*
 * Break a long statement into multiple lines (to not hit 80-char).
 */
asm volatile("int $0x80" : "=a"(res) : "a"(1), 
            "d"((unsigned long)"Hello from syscall\n\r"), 
            "D"((unsigned long)0) : "rcx", "r11", "memory");

/*
 * We use 'CamelCase' for everything.
 */

struct Data;
size_t bufferSize;
char *mimeType();

/*
 * main(), argc, and argv do not follow our naming scheme.
 */
int main(int argc, char *argv[]) {

}

static char *Function(int a, int b, float c, int d) {
	/*
	 * Declare any variables used within the function at the top of the
	 * function code. Sort bu size and then by alphabetical order.
	 * Avoid initalizing variables in the declarations.
	 */

	 extern u_char one;
	 extern char two;
	 struct foo three, *four;
	 double five;
	 int *six, seven;
	 char *eight, *nine, ten;
}

/*
 * Lastly, print out a copy of the GNU coding standards and *burn it*.
 * Do not read it, *burn it*, it doesn't deserve to be read.
 */
